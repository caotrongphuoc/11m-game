#include "ak.h"
#include "message.h"
#include "port.h"

#include "app.h"
#include "task_list.h"

#include "em_game_goal.h"

static em_game_goal_ball_t s_ball;
static em_game_goal_keeper_t s_keeper;
static bool s_ball_ready;
static bool s_keeper_ready;
static bool s_resolved;

static void em_game_goal_reset()
{
	s_ball.zone = EM_GAME_GOAL_ZONE_NONE;
	s_ball.is_wide = 0;
	s_keeper.zone = EM_GAME_GOAL_ZONE_NONE;
	s_ball_ready = false;
	s_keeper_ready = false;
	s_resolved = false;
}

static void em_game_goal_resolve()
{
	em_game_goal_result_msg_t result_msg;

	if (!s_ball_ready || !s_keeper_ready)
	{
		return;
	}

	if (s_ball.is_wide != 0)
	{
		result_msg.result = EM_GAME_GOAL_RESULT_MISS;
	}
	else if (s_ball.zone == s_keeper.zone)
	{
		result_msg.result = EM_GAME_GOAL_RESULT_SAVE;
	}
	else
	{
		result_msg.result = EM_GAME_GOAL_RESULT_GOAL;
	}

	s_resolved = true;
	s_ball_ready = false;
	s_keeper_ready = false;
	task_post_common_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_HIT_RESULT,
	                     (uint8_t*)&result_msg, sizeof(result_msg));
}

void em_game_goal_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_GOAL_SETUP:
	case EM_GAME_GOAL_RESET:
		em_game_goal_reset();
		break;

	case EM_GAME_GOAL_BALL_ARRIVED:
		if (!s_resolved && !s_ball_ready &&
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
				s_ball = *ball;
				s_ball_ready = true;
				em_game_goal_resolve();
			}
		}
		break;

	case EM_GAME_GOAL_KEEPER_READY:
		if (!s_resolved && !s_keeper_ready &&
		    (get_data_len_common_msg(msg) == sizeof(em_game_goal_keeper_t)))
		{
			em_game_goal_keeper_t* keeper =
			    (em_game_goal_keeper_t*)get_data_common_msg(msg);

			if ((keeper->zone >= EM_GAME_GOAL_ZONE_LEFT) &&
			    (keeper->zone <= EM_GAME_GOAL_ZONE_RIGHT))
			{
				s_keeper = *keeper;
				s_keeper_ready = true;
				em_game_goal_resolve();
			}
		}
		break;

	default:
		break;
	}
}
