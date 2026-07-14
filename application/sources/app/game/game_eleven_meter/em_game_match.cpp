#include "fsm.h"
#include "port.h"
#include "message.h"

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
	(void)msg;
}

static void em_game_match_state_shooter_wait(ak_msg_t* msg)
{
	(void)msg;
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
