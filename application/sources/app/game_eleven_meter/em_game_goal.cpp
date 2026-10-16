#include "em_game_goal.h"

/*****************************************************************************/
/* Variable declaration - Goal */
/*****************************************************************************/

em_game_goal_t em_game_goal = {};

/*****************************************************************************/
/* Private helpers - Goal */
/*****************************************************************************/

static void em_game_goal_reset()
{
	em_game_goal.ball.zone = EM_GAME_GOAL_ZONE_NONE;
	em_game_goal.ball.is_wide = 0;
	em_game_goal.keeper.zone = EM_GAME_GOAL_ZONE_NONE;
	em_game_goal.ball_ready = false;
	em_game_goal.keeper_ready = false;
	em_game_goal.resolved = false;
}

static void em_game_goal_resolve()
{
	em_game_goal_result_msg_t result_msg;

	if (!em_game_goal.ball_ready || !em_game_goal.keeper_ready)
	{
		return;
	}

	if (em_game_goal.ball.is_wide != 0)
	{
		result_msg.result = EM_GAME_GOAL_RESULT_MISS;
	}
	else if (em_game_goal.ball.zone == em_game_goal.keeper.zone)
	{
		result_msg.result = EM_GAME_GOAL_RESULT_SAVE;
	}
	else
	{
		result_msg.result = EM_GAME_GOAL_RESULT_GOAL;
	}

	em_game_goal.resolved = true;
	em_game_goal.ball_ready = false;
	em_game_goal.keeper_ready = false;
	task_post_common_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_HIT_RESULT,
	                     (uint8_t*)&result_msg, sizeof(result_msg));
}

/*****************************************************************************/
/* Handle - Goal */
/*****************************************************************************/

void em_game_goal_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_GOAL_SETUP:
		APP_DBG_SIG("EM_GAME_GOAL_SETUP\n");
		em_game_goal_reset();
		break;

	case EM_GAME_GOAL_RESET:
		APP_DBG_SIG("EM_GAME_GOAL_RESET\n");
		em_game_goal_reset();
		break;

	case EM_GAME_GOAL_BALL_ARRIVED:
		APP_DBG_SIG("EM_GAME_GOAL_BALL_ARRIVED\n");
		if (!em_game_goal.resolved && !em_game_goal.ball_ready &&
		    (get_data_len_common_msg(msg) == sizeof(em_game_goal_ball_t)))
		{
			em_game_goal_ball_t* ball =
			    (em_game_goal_ball_t*)get_data_common_msg(msg);

			if (((ball->is_wide == 1) &&
			     (ball->zone == EM_GAME_GOAL_ZONE_NONE)) ||
			    ((ball->is_wide == 0) &&
			     (ball->zone >= EM_GAME_GOAL_ZONE_LEFT) &&
			     (ball->zone <= EM_GAME_GOAL_ZONE_RIGHT)))
			{
				em_game_goal.ball = *ball;
				em_game_goal.ball_ready = true;
				em_game_goal_resolve();
			}
		}
		break;

	case EM_GAME_GOAL_KEEPER_READY:
		APP_DBG_SIG("EM_GAME_GOAL_KEEPER_READY\n");
		if (!em_game_goal.resolved && !em_game_goal.keeper_ready &&
		    (get_data_len_common_msg(msg) == sizeof(em_game_goal_keeper_t)))
		{
			em_game_goal_keeper_t* keeper =
			    (em_game_goal_keeper_t*)get_data_common_msg(msg);

			if ((keeper->zone >= EM_GAME_GOAL_ZONE_LEFT) &&
			    (keeper->zone <= EM_GAME_GOAL_ZONE_RIGHT))
			{
				em_game_goal.keeper = *keeper;
				em_game_goal.keeper_ready = true;
				em_game_goal_resolve();
			}
		}
		break;

	default:
		break;
	}
}
