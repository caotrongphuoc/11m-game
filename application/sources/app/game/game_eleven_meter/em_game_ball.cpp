#include "ak.h"
#include "port.h"
#include "message.h"

#include "app.h"
#include "task_list.h"

#include "em_game_ball.h"
#include "em_game_types.h"

void em_game_ball_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_BALL_KICK:
	{
		/*
		 * Temporary Phase 3 loopback. Replace with trajectory and hit
		 * resolution when the ball and goal tasks are implemented.
		 */
		uint8_t result = (uint8_t)EM_GAME_RESULT_GOAL;
		task_post_common_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_HIT_RESULT, &result, sizeof(result));
		break;
	}

	default:
		break;
	}
}
