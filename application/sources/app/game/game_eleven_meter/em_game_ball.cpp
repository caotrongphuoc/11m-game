#include "ak.h"
#include "message.h"
#include "port.h"
#include "timer.h"

#include "app.h"
#include "task_list.h"

#include "em_game_ball.h"
#include "em_game_goal.h"
#include "em_game_keeper.h"
#include "em_game_types.h"

em_game_ball_t em_game_ball;

static void em_game_ball_reset()
{
	timer_remove_attr(EM_GAME_BALL_ID, EM_GAME_BALL_ANIM_TICK);

	em_game_ball.x = EM_GAME_BALL_START_X;
	em_game_ball.y = EM_GAME_BALL_START_Y;
	em_game_ball.target_x = EM_GAME_BALL_START_X;
	em_game_ball.target_y = EM_GAME_BALL_START_Y;
	em_game_ball.frame = 0;
	em_game_ball.visible = false;
	em_game_ball.moving = false;
	em_game_ball.wide = false;
	em_game_ball.direction = EM_GAME_DIRECTION_NONE;

	task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_DISPLAY_REFRESH);
}

static int16_t em_game_ball_get_target_x(em_game_direction_t direction,
                                         bool wide)
{
	if (wide)
	{
		return (direction == EM_GAME_DIRECTION_LEFT)
		           ? EM_GAME_BALL_TARGET_WIDE_LEFT_X
		           : EM_GAME_BALL_TARGET_WIDE_RIGHT_X;
	}

	switch (direction)
	{
	case EM_GAME_DIRECTION_LEFT:
		return EM_GAME_BALL_TARGET_LEFT_X;

	case EM_GAME_DIRECTION_CENTER:
		return EM_GAME_BALL_TARGET_CENTER_X;

	case EM_GAME_DIRECTION_RIGHT:
		return EM_GAME_BALL_TARGET_RIGHT_X;

	default:
		return EM_GAME_BALL_START_X;
	}
}

static void em_game_ball_start(const em_game_ball_kick_t* kick)
{
	em_game_ball.x = EM_GAME_BALL_START_X;
	em_game_ball.y = EM_GAME_BALL_START_Y;
	em_game_ball.direction = (em_game_direction_t)kick->direction;
	em_game_ball.wide = (kick->is_wide != 0);
	em_game_ball.target_x =
	    em_game_ball_get_target_x(em_game_ball.direction, em_game_ball.wide);
	em_game_ball.target_y = EM_GAME_BALL_TARGET_Y;
	em_game_ball.frame = 0;
	em_game_ball.visible = true;
	em_game_ball.moving = true;

	timer_set(EM_GAME_BALL_ID, EM_GAME_BALL_ANIM_TICK,
	          EM_GAME_BALL_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
	task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_DISPLAY_REFRESH);
}

static void em_game_ball_advance()
{
	if (!em_game_ball.moving)
	{
		return;
	}

	em_game_ball.frame++;
	em_game_ball.x =
	    EM_GAME_BALL_START_X +
	    ((em_game_ball.target_x - EM_GAME_BALL_START_X) * em_game_ball.frame) /
	        EM_GAME_BALL_STEP_COUNT;
	em_game_ball.y =
	    EM_GAME_BALL_START_Y +
	    ((em_game_ball.target_y - EM_GAME_BALL_START_Y) * em_game_ball.frame) /
	        EM_GAME_BALL_STEP_COUNT;
	task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_DISPLAY_REFRESH);

	if (em_game_ball.frame < EM_GAME_BALL_STEP_COUNT)
	{
		return;
	}

	em_game_ball.x = em_game_ball.target_x;
	em_game_ball.y = em_game_ball.target_y;
	em_game_ball.moving = false;
	timer_remove_attr(EM_GAME_BALL_ID, EM_GAME_BALL_ANIM_TICK);

	if (em_game_ball.wide)
	{
		uint8_t result_payload = (uint8_t)EM_GAME_RESULT_MISS;

		task_post_common_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_HIT_RESULT,
		                     &result_payload, sizeof(result_payload));
	}
	else
	{
		em_game_goal_hit_t hit;

		hit.ball_direction = (uint8_t)em_game_ball.direction;
		hit.keeper_direction = (uint8_t)em_game_keeper_get_direction();

		task_post_common_msg(EM_GAME_GOAL_ID, EM_GAME_GOAL_CHECK_HIT,
		                     (uint8_t*)&hit, sizeof(hit));
	}
}

void em_game_ball_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_BALL_SETUP:
	case EM_GAME_BALL_RESET:
		em_game_ball_reset();
		break;

	case EM_GAME_BALL_KICK:
	{
		if (get_data_len_common_msg(msg) == sizeof(em_game_ball_kick_t))
		{
			em_game_ball_start((em_game_ball_kick_t*)get_data_common_msg(msg));
		}
		break;
	}

	case EM_GAME_BALL_ANIM_TICK:
		em_game_ball_advance();
		break;

	default:
		break;
	}
}
