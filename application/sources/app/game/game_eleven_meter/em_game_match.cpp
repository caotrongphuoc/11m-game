#include "ak.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"

#include "app.h"
#include "task_list.h"

#include "em_game_ball.h"
#include "em_game_keeper.h"
#include "em_game_match.h"
#include "em_game_match_state.h"
#include "em_game_shooter.h"

static em_game_match_state_t s_match;
static bool s_match_initialized = false;

enum
{
	EM_GAME_STATE_MENU,
	EM_GAME_STATE_ROUND_START,
	EM_GAME_STATE_SHOOTER_WAIT,
	EM_GAME_STATE_REVEAL,
	EM_GAME_STATE_ROUND_END,
	EM_GAME_STATE_GAME_OVER,
};

static uint8_t s_state;

static void em_game_match_cycle_difficulty_left()
{
	if (s_match.difficulty == EM_GAME_DIFFICULTY_EASY)
	{
		s_match.difficulty = EM_GAME_DIFFICULTY_HARD;
	}
	else
	{
		s_match.difficulty =
		    (em_game_difficulty_t)(s_match.difficulty - 1);
	}
}

static void em_game_match_cycle_difficulty_right()
{
	if (s_match.difficulty == EM_GAME_DIFFICULTY_HARD)
	{
		s_match.difficulty = EM_GAME_DIFFICULTY_EASY;
	}
	else
	{
		s_match.difficulty =
		    (em_game_difficulty_t)(s_match.difficulty + 1);
	}
}

static uint32_t em_game_match_xorshift32(uint32_t* state)
{
	uint32_t value = *state;

	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	*state = value;

	return value;
}

static void em_game_match_refresh_display()
{
	em_game_view_t view;

	s_match.ball = em_game_ball;
	s_match.keeper = em_game_keeper;
	s_match.shooter = em_game_shooter;

	em_game_match_state_build_view(&s_match, &view);
	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_REFRESH,
	                     (uint8_t*)&view, sizeof(view));
}

static void em_game_match_handle_hit_result(ak_msg_t* msg)
{
	em_game_result_t result;
	uint8_t buzzer_signal;

	if (get_data_len_common_msg(msg) != sizeof(uint8_t))
	{
		return;
	}

	result = (em_game_result_t)(*get_data_common_msg(msg));

	switch (result)
	{
	case EM_GAME_RESULT_GOAL:
		buzzer_signal = EM_GAME_BUZZER_GOAL;
		break;

	case EM_GAME_RESULT_SAVE:
		buzzer_signal = EM_GAME_BUZZER_SAVE;
		break;

	case EM_GAME_RESULT_MISS:
		buzzer_signal = EM_GAME_BUZZER_MISS;
		break;

	default:
		return;
	}

	s_match.last_result = result;
	task_post_pure_msg(AC_TASK_BUZZER_ID, buzzer_signal);
	em_game_scoreboard_record_result(&s_match.scoreboard, result);
	timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_ROUND_END,
	          EM_GAME_MATCH_ROUND_END_INTERVAL, TIMER_ONE_SHOT);
	s_state = EM_GAME_STATE_ROUND_END;
	em_game_match_refresh_display();
}

static void em_game_match_finish_match()
{
	em_game_winner_t winner =
	    em_game_scoreboard_evaluate_winner(&s_match.scoreboard);

	em_game_match_refresh_display();
	task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_GAME_OVER);

	if (winner == EM_GAME_WINNER_PLAYER)
	{
		task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_WIN);
	}
	else
	{
		task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_LOSE);
	}

	s_state = EM_GAME_STATE_GAME_OVER;
}

static void em_game_match_start_kick(em_game_direction_t direction)
{
	em_game_ball_kick_t ball_kick;
	em_game_keeper_ai_pick_t keeper_pick;
	uint32_t reaction_time;
	uint8_t miss_chance;
	uint8_t miss_roll;
	uint8_t direction_payload = (uint8_t)direction;

	reaction_time = sys_ctrl_millis() - s_match.countdown_start_tick;
	miss_chance = EM_GAME_MATCH_MISS_BASE_CHANCE;

	if (reaction_time < EM_GAME_MATCH_FAST_REACTION_INTERVAL)
	{
		miss_chance += EM_GAME_MATCH_MISS_FAST_BONUS;
	}

	if (miss_chance > EM_GAME_MATCH_MISS_MAX_CHANCE)
	{
		miss_chance = EM_GAME_MATCH_MISS_MAX_CHANCE;
	}

	miss_roll = em_game_match_xorshift32(&s_match.random_seed) % 100;

	s_match.countdown_seconds = 0;
	ball_kick.direction = (uint8_t)direction;
	ball_kick.is_wide = (miss_roll < miss_chance) ? 1 : 0;
	keeper_pick.shooter_direction = (uint8_t)direction;
	keeper_pick.difficulty = (uint8_t)s_match.difficulty;

	task_post_common_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_KICK,
	                     &direction_payload, sizeof(direction_payload));
	task_post_common_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_AI_PICK,
	                     (uint8_t*)&keeper_pick, sizeof(keeper_pick));
	task_post_common_msg(EM_GAME_BALL_ID, EM_GAME_BALL_KICK,
	                     (uint8_t*)&ball_kick, sizeof(ball_kick));
	task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_KICK);

	s_state = EM_GAME_STATE_REVEAL;
	em_game_match_refresh_display();
}

void em_game_match_handle(ak_msg_t* msg)
{
	if (!s_match_initialized)
	{
		em_game_match_state_init(&s_match);
		s_state = EM_GAME_STATE_MENU;
		task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
		em_game_match_refresh_display();
		s_match_initialized = true;
	}

	switch (msg->sig)
	{
	case EM_GAME_MATCH_SETUP:
		if (s_state == EM_GAME_STATE_ROUND_START)
		{
			em_game_scoreboard_advance_round(&s_match.scoreboard);
			s_match.ball.direction = EM_GAME_DIRECTION_NONE;
			s_match.keeper.direction = EM_GAME_DIRECTION_NONE;
			s_match.shooter.direction = EM_GAME_DIRECTION_NONE;
			s_match.last_result = EM_GAME_RESULT_NONE;
			task_post_pure_msg(EM_GAME_BALL_ID, EM_GAME_BALL_RESET);
			task_post_pure_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_RESET);
			task_post_pure_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_RESET);
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START_ROUND);
		}
		break;

	case EM_GAME_MATCH_START_ROUND:
		if (s_state == EM_GAME_STATE_ROUND_START)
		{
			s_match.countdown_start_tick = sys_ctrl_millis();
			s_match.countdown_seconds = 3;
			task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_PENALTY);
			timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT,
			          EM_GAME_MATCH_SELECTION_TIMEOUT, TIMER_ONE_SHOT);
			timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_COUNTDOWN_TICK,
			          EM_GAME_MATCH_COUNTDOWN_TICK_INTERVAL, TIMER_PERIODIC);
			s_state = EM_GAME_STATE_SHOOTER_WAIT;
			em_game_match_refresh_display();
		}
		break;

	case EM_GAME_MATCH_START:
		if (s_state == EM_GAME_STATE_MENU)
		{
			if (s_match.random_seed == 0)
			{
				s_match.random_seed = sys_ctrl_millis();
				if (s_match.random_seed == 0)
				{
					s_match.random_seed = EM_GAME_MATCH_RANDOM_FALLBACK;
				}
			}

			em_game_scoreboard_reset(&s_match.scoreboard);
			s_state = EM_GAME_STATE_ROUND_START;
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		}
		break;

	case EM_GAME_MATCH_MENU_LEFT:
		if (s_state == EM_GAME_STATE_MENU)
		{
			em_game_match_cycle_difficulty_left();
			em_game_match_refresh_display();
		}
		break;

	case EM_GAME_MATCH_MENU_RIGHT:
		if (s_state == EM_GAME_STATE_MENU)
		{
			em_game_match_cycle_difficulty_right();
			em_game_match_refresh_display();
		}
		break;

	case EM_GAME_MATCH_KICK_LEFT:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_COUNTDOWN_TICK);
			em_game_match_start_kick(EM_GAME_DIRECTION_LEFT);
		}
		break;

	case EM_GAME_MATCH_KICK_CENTER:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_COUNTDOWN_TICK);
			em_game_match_start_kick(EM_GAME_DIRECTION_CENTER);
		}
		break;

	case EM_GAME_MATCH_KICK_RIGHT:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_COUNTDOWN_TICK);
			em_game_match_start_kick(EM_GAME_DIRECTION_RIGHT);
		}
		break;

	case EM_GAME_MATCH_RETRY:
		if (s_state == EM_GAME_STATE_GAME_OVER)
		{
			em_game_match_state_reset(&s_match);
			s_state = EM_GAME_STATE_ROUND_START;
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		}
		break;

	case EM_GAME_MATCH_HOME:
		if (s_state == EM_GAME_STATE_GAME_OVER)
		{
			em_game_match_state_reset(&s_match);
			s_state = EM_GAME_STATE_MENU;
			task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
		}
		break;

	case EM_GAME_MATCH_SHOOTER_TIMEOUT:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_COUNTDOWN_TICK);
			em_game_match_start_kick(EM_GAME_DIRECTION_CENTER);
		}
		break;

	case EM_GAME_MATCH_COUNTDOWN_TICK:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			uint32_t elapsed =
			    sys_ctrl_millis() - s_match.countdown_start_tick;
			uint8_t elapsed_seconds = elapsed / 1000;

			s_match.countdown_seconds =
			    (elapsed_seconds < 3) ? (3 - elapsed_seconds) : 0;
			em_game_match_refresh_display();
		}
		break;

	case EM_GAME_MATCH_DISPLAY_REFRESH:
		em_game_match_refresh_display();
		break;

	case EM_GAME_MATCH_HIT_RESULT:
		if (s_state == EM_GAME_STATE_REVEAL)
		{
			em_game_match_handle_hit_result(msg);
		}
		break;

	case EM_GAME_MATCH_ROUND_END:
		if (s_state == EM_GAME_STATE_ROUND_END)
		{
			if (em_game_scoreboard_is_complete(&s_match.scoreboard))
			{
				em_game_match_finish_match();
			}
			else
			{
				s_state = EM_GAME_STATE_ROUND_START;
				task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
			}
		}
		break;

	default:
		break;
	}
}
