#include "ak.h"
#include "port.h"
#include "message.h"

#include "app.h"
#include "task_list.h"

#include "em_game_goal.h"
#include "em_game_types.h"

static void em_game_goal_check_hit(ak_msg_t* msg)
{
	em_game_goal_hit_t* hit;
	em_game_result_t result;
	uint8_t result_payload;

	if (get_data_len_common_msg(msg) != sizeof(em_game_goal_hit_t))
	{
		return;
	}

	hit = (em_game_goal_hit_t*)get_data_common_msg(msg);

	if ((hit->ball_direction < EM_GAME_DIRECTION_LEFT) ||
	    (hit->ball_direction > EM_GAME_DIRECTION_RIGHT) ||
	    (hit->keeper_direction < EM_GAME_DIRECTION_LEFT) ||
	    (hit->keeper_direction > EM_GAME_DIRECTION_RIGHT))
	{
		return;
	}

	result = (hit->ball_direction == hit->keeper_direction)
	             ? EM_GAME_RESULT_SAVE
	             : EM_GAME_RESULT_GOAL;
	result_payload = (uint8_t)result;

	task_post_common_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_HIT_RESULT,
	                     &result_payload, sizeof(result_payload));
}

void em_game_goal_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_GOAL_CHECK_HIT:
		em_game_goal_check_hit(msg);
		break;

	case EM_GAME_GOAL_SETUP:
	case EM_GAME_GOAL_RESET:
		break;

	default:
		break;
	}
}
