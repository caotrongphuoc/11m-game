#include "fsm.h"
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

static void em_game_match_state_menu(ak_msg_t* msg);
static void em_game_match_state_round_start(ak_msg_t* msg);
static void em_game_match_state_shooter_wait(ak_msg_t* msg);
static void em_game_match_state_reveal(ak_msg_t* msg);
static void em_game_match_state_round_end(ak_msg_t* msg);
static void em_game_match_state_game_over(ak_msg_t* msg);

static void em_game_match_start_kick(em_game_direction_t direction)
{
	uint8_t direction_payload = (uint8_t)direction;

	s_match.pending_direction = direction;

	task_post_common_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_KICK, &direction_payload, sizeof(direction_payload));
	task_post_pure_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_AI_PICK);
	task_post_common_msg(EM_GAME_BALL_ID, EM_GAME_BALL_KICK, &direction_payload, sizeof(direction_payload));
	task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_KICK);

	FSM_TRAN(&s_match, em_game_match_state_reveal);
}

static void em_game_match_state_menu(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_MATCH_INPUT_CENTER:
		em_game_scoreboard_reset(&s_match.scoreboard);
		FSM_TRAN(&s_match, em_game_match_state_round_start);
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		break;

	default:
		break;
	}
}

static void em_game_match_state_round_start(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_MATCH_SETUP:
		em_game_scoreboard_advance_round(&s_match.scoreboard);
		s_match.ball.direction = EM_GAME_DIRECTION_NONE;
		s_match.keeper.direction = EM_GAME_DIRECTION_NONE;
		s_match.shooter.direction = EM_GAME_DIRECTION_NONE;
		s_match.pending_direction = EM_GAME_DIRECTION_NONE;
		s_match.last_result = EM_GAME_RESULT_NONE;
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START_ROUND);
		break;

	case EM_GAME_MATCH_START_ROUND:
		s_match.countdown_start_tick = sys_ctrl_millis();
		s_match.countdown_seconds = 3;
		task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_PENALTY);
		timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT, EM_GAME_MATCH_SELECTION_TIMEOUT, TIMER_ONE_SHOT);
		FSM_TRAN(&s_match, em_game_match_state_shooter_wait);
		break;

	default:
		break;
	}
}

static void em_game_match_state_shooter_wait(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_MATCH_INPUT_LEFT:
		timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
		em_game_match_start_kick(EM_GAME_DIRECTION_LEFT);
		break;

	case EM_GAME_MATCH_INPUT_CENTER:
		timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
		em_game_match_start_kick(EM_GAME_DIRECTION_CENTER);
		break;

	case EM_GAME_MATCH_INPUT_RIGHT:
		timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
		em_game_match_start_kick(EM_GAME_DIRECTION_RIGHT);
		break;

	case EM_GAME_MATCH_SHOOTER_TIMEOUT:
		em_game_match_start_kick(EM_GAME_DIRECTION_CENTER);
		break;

	case EM_GAME_MATCH_RESET:
		timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_SHOOTER_TIMEOUT);
		FSM_TRAN(&s_match, em_game_match_state_menu);
		break;

	default:
		break;
	}
}

static void em_game_match_state_reveal(ak_msg_t* msg)
{
	(void)msg;
}

static void em_game_match_state_round_end(ak_msg_t* msg)
{
	(void)msg;
}

static void em_game_match_state_game_over(ak_msg_t* msg)
{
	(void)msg;
}

void em_game_match_handle(ak_msg_t* msg)
{
	if (!s_match_initialized)
	{
		em_game_match_state_init(&s_match);
		FSM(&s_match, em_game_match_state_menu);
		task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
		s_match_initialized = true;
	}

	fsm_dispatch((fsm_t*)&s_match, msg);
}
