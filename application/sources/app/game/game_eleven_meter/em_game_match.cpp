#include "ak.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"

#include "app.h"
#include "task_list.h"

#include "em_game_match.h"
#include "em_game_match_state.h"

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
	timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_ROUND_END, EM_GAME_MATCH_ROUND_END_INTERVAL, TIMER_ONE_SHOT);
	s_state = EM_GAME_STATE_ROUND_END;
}

static void em_game_match_finish_match()
{
	em_game_winner_t winner = em_game_scoreboard_evaluate_winner(&s_match.scoreboard);

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
	uint8_t direction_payload = (uint8_t)direction;

	s_match.pending_direction = direction;

	task_post_common_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_KICK, &direction_payload, sizeof(direction_payload));
	task_post_pure_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_AI_PICK);
	task_post_common_msg(EM_GAME_BALL_ID, EM_GAME_BALL_KICK, &direction_payload, sizeof(direction_payload));
	task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_KICK);

	s_state = EM_GAME_STATE_REVEAL;
}

void em_game_match_handle(ak_msg_t* msg)
{
	if (!s_match_initialized)
	{
		em_game_match_state_init(&s_match);
		s_state = EM_GAME_STATE_MENU;
		task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
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
			s_match.pending_direction = EM_GAME_DIRECTION_NONE;
			s_match.last_result = EM_GAME_RESULT_NONE;
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START_ROUND);
		}
		break;

	case EM_GAME_MATCH_START_ROUND:
		if (s_state == EM_GAME_STATE_ROUND_START)
		{
			s_match.countdown_start_tick = sys_ctrl_millis();
			s_match.countdown_seconds = 3;
			task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_PENALTY);
			timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT, EM_GAME_MATCH_SELECTION_TIMEOUT, TIMER_ONE_SHOT);
			s_state = EM_GAME_STATE_SHOOTER_WAIT;
		}
		break;

	case EM_GAME_MATCH_INPUT_LEFT:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
			em_game_match_start_kick(EM_GAME_DIRECTION_LEFT);
		}
		else if (s_state == EM_GAME_STATE_GAME_OVER)
		{
			s_state = EM_GAME_STATE_MENU;
			task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
		}
		break;

	case EM_GAME_MATCH_INPUT_CENTER:
		if (s_state == EM_GAME_STATE_MENU)
		{
			em_game_scoreboard_reset(&s_match.scoreboard);
			s_state = EM_GAME_STATE_ROUND_START;
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		}
		else if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
			em_game_match_start_kick(EM_GAME_DIRECTION_CENTER);
		}
		else if (s_state == EM_GAME_STATE_GAME_OVER)
		{
			em_game_match_state_reset(&s_match);
			s_state = EM_GAME_STATE_MENU;
			task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
		}
		break;

	case EM_GAME_MATCH_INPUT_RIGHT:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
			em_game_match_start_kick(EM_GAME_DIRECTION_RIGHT);
		}
		break;

	case EM_GAME_MATCH_SHOOTER_TIMEOUT:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			em_game_match_start_kick(EM_GAME_DIRECTION_CENTER);
		}
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

	case EM_GAME_MATCH_RESET:
		if (s_state == EM_GAME_STATE_SHOOTER_WAIT)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
			s_state = EM_GAME_STATE_MENU;
		}
		else if (s_state == EM_GAME_STATE_REVEAL)
		{
			s_state = EM_GAME_STATE_MENU;
		}
		else if (s_state == EM_GAME_STATE_ROUND_END)
		{
			timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_ROUND_END);
			s_state = EM_GAME_STATE_MENU;
		}
		break;

	default:
		break;
	}
}
