#include "ak.h"
#include "message.h"
#include "port.h"
#include "timer.h"

#include "app.h"
#include "app_eeprom.h"
#include "task_list.h"

#include "em_game_goal.h"
#include "em_game_keeper.h"
#include "em_game_match.h"
#include "em_game_scoreboard.h"

static em_game_scoreboard_t s_scoreboard;
static em_game_match_difficulty_t s_difficulty =
    EM_GAME_MATCH_DIFFICULTY_NORMAL;
static em_game_goal_result_t s_last_result = EM_GAME_GOAL_RESULT_NONE;
static em_game_match_state_t s_state = EM_GAME_MATCH_STATE_MENU;
static uint8_t s_best_goals;
static em_game_match_difficulty_t s_best_difficulty =
    EM_GAME_MATCH_DIFFICULTY_EASY;
static uint8_t s_countdown_seconds;
static uint16_t s_state_elapsed_ms;
static bool s_initialized;

static void em_game_match_set_state(em_game_match_state_t state)
{
	s_state = state;
	s_state_elapsed_ms = 0;
}

static void em_game_match_publish_view()
{
	em_game_match_view_t view;

	view.round = s_scoreboard.round;
	view.goals = s_scoreboard.goals;
	view.saves = s_scoreboard.saves;
	view.misses = s_scoreboard.misses;
	view.countdown = s_countdown_seconds;
	view.state = (uint8_t)s_state;
	view.difficulty = (uint8_t)s_difficulty;
	view.best_goals = s_best_goals;
	view.best_difficulty = (uint8_t)s_best_difficulty;
	view.last_result = (uint8_t)s_last_result;
	view.winner = (uint8_t)s_scoreboard.winner;

	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_MATCH,
	                     (uint8_t*)&view, sizeof(view));
}

static void em_game_match_set_keeper_difficulty()
{
	uint8_t difficulty = (uint8_t)s_difficulty;

	task_post_common_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_SET_DIFFICULTY,
	                     &difficulty, sizeof(difficulty));
}

static void em_game_match_cycle_difficulty_left()
{
	if (s_difficulty == EM_GAME_MATCH_DIFFICULTY_EASY)
	{
		s_difficulty = EM_GAME_MATCH_DIFFICULTY_HARD;
	}
	else
	{
		s_difficulty = (em_game_match_difficulty_t)(s_difficulty - 1);
	}

	em_game_match_set_keeper_difficulty();
	em_game_match_publish_view();
}

static void em_game_match_cycle_difficulty_right()
{
	if (s_difficulty == EM_GAME_MATCH_DIFFICULTY_HARD)
	{
		s_difficulty = EM_GAME_MATCH_DIFFICULTY_EASY;
	}
	else
	{
		s_difficulty = (em_game_match_difficulty_t)(s_difficulty + 1);
	}

	em_game_match_set_keeper_difficulty();
	em_game_match_publish_view();
}

static void em_game_match_reset()
{
	em_game_scoreboard_reset(&s_scoreboard);
	s_last_result = EM_GAME_GOAL_RESULT_NONE;
	s_countdown_seconds = 0;
}

static void em_game_match_handle_hit_result(ak_msg_t* msg)
{
	em_game_goal_result_t result;
	uint8_t buzzer_signal;

	if (get_data_len_common_msg(msg) != sizeof(uint8_t))
	{
		return;
	}

	result = (em_game_goal_result_t)(*get_data_common_msg(msg));

	switch (result)
	{
	case EM_GAME_GOAL_RESULT_GOAL:
		buzzer_signal = EM_GAME_BUZZER_GOAL;
		break;

	case EM_GAME_GOAL_RESULT_SAVE:
		buzzer_signal = EM_GAME_BUZZER_SAVE;
		break;

	case EM_GAME_GOAL_RESULT_MISS:
		buzzer_signal = EM_GAME_BUZZER_MISS;
		break;

	default:
		return;
	}

	s_last_result = result;
	em_game_scoreboard_record_result(&s_scoreboard, result);
	task_post_pure_msg(AC_TASK_BUZZER_ID, buzzer_signal);
	em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_END);
	em_game_match_publish_view();
}

static void em_game_match_finish()
{
	em_game_score_record_t score_record;

	em_game_scoreboard_evaluate_winner(&s_scoreboard);

	if (s_scoreboard.goals > s_best_goals)
	{
		s_best_goals = s_scoreboard.goals;
		s_best_difficulty = s_difficulty;

		init_score_record(&score_record);
		score_record.best_goals = s_best_goals;
		score_record.best_difficulty = (uint8_t)s_best_difficulty;
		save_score_record(&score_record);
	}

	timer_remove_attr(AC_TASK_DISPLAY_ID, EM_GAME_TIME_TICK);
	em_game_match_set_state(EM_GAME_MATCH_STATE_RIP);
	em_game_match_publish_view();
	task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_RIP);
	timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_RIP_TIMEOUT,
	          EM_GAME_MATCH_RIP_DURATION, TIMER_ONE_SHOT);
}

static void em_game_match_show_game_over()
{
	timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_RIP_TIMEOUT);
	em_game_match_set_state(EM_GAME_MATCH_STATE_GAME_OVER);
	em_game_match_publish_view();
	task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_GAME_OVER);

	if (s_scoreboard.winner == EM_GAME_SCOREBOARD_WINNER_PLAYER)
	{
		task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_WIN);
	}
	else
	{
		task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_LOSE);
	}
}

static void em_game_match_request_kick(uint8_t shooter_signal)
{
	s_countdown_seconds = 0;
	em_game_match_set_state(EM_GAME_MATCH_STATE_REVEAL);
	task_post_pure_msg(EM_GAME_SHOOTER_ID, shooter_signal);
	em_game_match_publish_view();
}

static void em_game_match_initialize()
{
	em_game_score_record_t score_record;

	em_game_match_reset();
	load_score_record(&score_record);
	s_best_goals = score_record.best_goals;

	if (score_record.best_difficulty <= EM_GAME_MATCH_DIFFICULTY_HARD)
	{
		s_best_difficulty =
		    (em_game_match_difficulty_t)score_record.best_difficulty;
	}

	em_game_match_set_state(EM_GAME_MATCH_STATE_MENU);
	em_game_match_set_keeper_difficulty();
	em_game_match_publish_view();
	task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
	s_initialized = true;
}

void em_game_match_handle(ak_msg_t* msg)
{
	if (!s_initialized)
	{
		em_game_match_initialize();
	}

	switch (msg->sig)
	{
	case EM_GAME_MATCH_SETUP:
		if (s_state == EM_GAME_MATCH_STATE_ROUND_START)
		{
			em_game_scoreboard_advance_round(&s_scoreboard);
			s_last_result = EM_GAME_GOAL_RESULT_NONE;

			if (s_scoreboard.round > 1)
			{
				task_post_pure_msg(EM_GAME_GOAL_ID, EM_GAME_GOAL_RESET);
				task_post_pure_msg(EM_GAME_BALL_ID, EM_GAME_BALL_RESET);
				task_post_pure_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_RESET);
				task_post_pure_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_RESET);
			}

			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START_ROUND);
			em_game_match_publish_view();
		}
		break;

	case EM_GAME_MATCH_START_ROUND:
		if (s_state == EM_GAME_MATCH_STATE_ROUND_START)
		{
			s_countdown_seconds = 3;
			em_game_match_set_state(EM_GAME_MATCH_STATE_SHOOTER_WAIT);
			if (s_scoreboard.round == 1)
			{
				task_post_pure_msg(AC_TASK_DISPLAY_ID,
				                   EM_GAME_DISPLAY_SHOW_PENALTY);
			}
			em_game_match_publish_view();
		}
		break;

	case EM_GAME_MATCH_START:
		if (s_state == EM_GAME_MATCH_STATE_MENU)
		{
			em_game_match_reset();
			em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_START);
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		}
		break;

	case EM_GAME_MATCH_MENU_LEFT:
		if (s_state == EM_GAME_MATCH_STATE_MENU)
		{
			em_game_match_cycle_difficulty_left();
		}
		break;

	case EM_GAME_MATCH_MENU_RIGHT:
		if (s_state == EM_GAME_MATCH_STATE_MENU)
		{
			em_game_match_cycle_difficulty_right();
		}
		break;

	case EM_GAME_MATCH_KICK_LEFT:
		if (s_state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			em_game_match_request_kick(EM_GAME_SHOOTER_REQUEST_KICK_LEFT);
		}
		break;

	case EM_GAME_MATCH_KICK_CENTER:
		if (s_state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			em_game_match_request_kick(EM_GAME_SHOOTER_REQUEST_KICK_CENTER);
		}
		break;

	case EM_GAME_MATCH_KICK_RIGHT:
		if (s_state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			em_game_match_request_kick(EM_GAME_SHOOTER_REQUEST_KICK_RIGHT);
		}
		break;

	case EM_GAME_MATCH_RETRY:
		if (s_state == EM_GAME_MATCH_STATE_GAME_OVER)
		{
			em_game_match_reset();
			em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_START);
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		}
		break;

	case EM_GAME_MATCH_HOME:
		if (s_state == EM_GAME_MATCH_STATE_GAME_OVER)
		{
			em_game_match_reset();
			em_game_match_set_state(EM_GAME_MATCH_STATE_MENU);
			em_game_match_publish_view();
			task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
		}
		break;

	case EM_GAME_MATCH_UPDATE:
		if (s_state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			uint8_t countdown_seconds;

			s_state_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (s_state_elapsed_ms >= EM_GAME_MATCH_SELECTION_TIMEOUT)
			{
				em_game_match_request_kick(
				    EM_GAME_SHOOTER_REQUEST_KICK_CENTER);
				break;
			}

			countdown_seconds =
			    (uint8_t)((EM_GAME_MATCH_SELECTION_TIMEOUT -
			               s_state_elapsed_ms + 999U) /
			              1000U);
			if (countdown_seconds != s_countdown_seconds)
			{
				s_countdown_seconds = countdown_seconds;
				em_game_match_publish_view();
			}
		}
		else if (s_state == EM_GAME_MATCH_STATE_ROUND_END)
		{
			s_state_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (s_state_elapsed_ms < EM_GAME_MATCH_ROUND_END_INTERVAL)
			{
				break;
			}

			if (em_game_scoreboard_is_complete(&s_scoreboard))
			{
				em_game_match_finish();
			}
			else
			{
				em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_START);
				task_post_pure_msg(EM_GAME_MATCH_ID,
				                   EM_GAME_MATCH_SETUP);
			}
		}
		break;

	case EM_GAME_MATCH_HIT_RESULT:
		if (s_state == EM_GAME_MATCH_STATE_REVEAL)
		{
			em_game_match_handle_hit_result(msg);
		}
		break;

	case EM_GAME_MATCH_RIP_TIMEOUT:
	case EM_GAME_MATCH_SKIP_RIP:
		if (s_state == EM_GAME_MATCH_STATE_RIP)
		{
			em_game_match_show_game_over();
		}
		break;

	default:
		break;
	}
}
