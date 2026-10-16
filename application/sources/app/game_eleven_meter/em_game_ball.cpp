#include "em_game_ball.h"

/*****************************************************************************/
/* Variable declaration - Ball */
/*****************************************************************************/

em_game_ball_t em_game_ball = {};

/*****************************************************************************/
/* Private helpers - Ball */
/*****************************************************************************/

static void em_game_ball_publish_view()
{
	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_BALL,
	                     (uint8_t*)&em_game_ball.view, sizeof(em_game_ball.view));
}

static int16_t em_game_ball_get_target_x(em_game_ball_target_t target)
{
	switch (target)
	{
	case EM_GAME_BALL_TARGET_LEFT:
		return EM_GAME_BALL_TARGET_LEFT_X;

	case EM_GAME_BALL_TARGET_CENTER:
		return EM_GAME_BALL_TARGET_CENTER_X;

	case EM_GAME_BALL_TARGET_RIGHT:
		return EM_GAME_BALL_TARGET_RIGHT_X;

	case EM_GAME_BALL_TARGET_WIDE_LEFT:
		return EM_GAME_BALL_TARGET_WIDE_LEFT_X;

	case EM_GAME_BALL_TARGET_WIDE_RIGHT:
		return EM_GAME_BALL_TARGET_WIDE_RIGHT_X;

	default:
		return EM_GAME_BALL_START_X;
	}
}

static void em_game_ball_reset()
{
	em_game_ball.view.x = EM_GAME_BALL_START_X;
	em_game_ball.view.y = EM_GAME_BALL_START_Y;
	em_game_ball.view.frame = 0;
	em_game_ball.view.visible = true;
	em_game_ball.view.moving = false;
	em_game_ball.view.target = EM_GAME_BALL_TARGET_NONE;
	em_game_ball.target_x = EM_GAME_BALL_START_X;
	em_game_ball.update_elapsed_ms = 0;
	em_game_ball.kick_started = false;

	em_game_ball_publish_view();
}

static void em_game_ball_start(const em_game_ball_kick_t* kick)
{
	em_game_ball_target_t target = (em_game_ball_target_t)kick->target;

	if (em_game_ball.kick_started || (target < EM_GAME_BALL_TARGET_LEFT) ||
	    (target > EM_GAME_BALL_TARGET_WIDE_RIGHT))
	{
		return;
	}

	em_game_ball.view.x = EM_GAME_BALL_START_X;
	em_game_ball.view.y = EM_GAME_BALL_START_Y;
	em_game_ball.view.frame = 0;
	em_game_ball.view.visible = true;
	em_game_ball.view.moving = true;
	em_game_ball.view.target = (uint8_t)target;
	em_game_ball.target_x = em_game_ball_get_target_x(target);
	em_game_ball.update_elapsed_ms = 0;
	em_game_ball.kick_started = true;

	em_game_ball_publish_view();
}

static void em_game_ball_report_arrival()
{
	em_game_goal_ball_t ball;

	switch ((em_game_ball_target_t)em_game_ball.view.target)
	{
	case EM_GAME_BALL_TARGET_LEFT:
		ball.zone = EM_GAME_GOAL_ZONE_LEFT;
		ball.is_wide = 0;
		break;

	case EM_GAME_BALL_TARGET_CENTER:
		ball.zone = EM_GAME_GOAL_ZONE_CENTER;
		ball.is_wide = 0;
		break;

	case EM_GAME_BALL_TARGET_RIGHT:
		ball.zone = EM_GAME_GOAL_ZONE_RIGHT;
		ball.is_wide = 0;
		break;

	case EM_GAME_BALL_TARGET_WIDE_LEFT:
	case EM_GAME_BALL_TARGET_WIDE_RIGHT:
	default:
		ball.zone = EM_GAME_GOAL_ZONE_NONE;
		ball.is_wide = 1;
		break;
	}

	task_post_common_msg(EM_GAME_GOAL_ID, EM_GAME_GOAL_BALL_ARRIVED,
	                     (uint8_t*)&ball, sizeof(ball));
}

static void em_game_ball_advance()
{
	if (!em_game_ball.view.moving)
	{
		return;
	}

	em_game_ball.view.frame++;
	em_game_ball.view.x = EM_GAME_BALL_START_X +
	                      ((em_game_ball.target_x - EM_GAME_BALL_START_X) * em_game_ball.view.frame) /
	                          EM_GAME_BALL_STEP_COUNT;
	em_game_ball.view.y = EM_GAME_BALL_START_Y +
	                      ((EM_GAME_BALL_TARGET_Y - EM_GAME_BALL_START_Y) * em_game_ball.view.frame) /
	                          EM_GAME_BALL_STEP_COUNT;

	if (em_game_ball.view.frame >= EM_GAME_BALL_STEP_COUNT)
	{
		em_game_ball.view.x = em_game_ball.target_x;
		em_game_ball.view.y = EM_GAME_BALL_TARGET_Y;
		em_game_ball.view.moving = false;
		em_game_ball_report_arrival();
	}

	em_game_ball_publish_view();
}

/*****************************************************************************/
/* Handle - Ball */
/*****************************************************************************/

void em_game_ball_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_BALL_SETUP:
		APP_DBG_SIG("EM_GAME_BALL_SETUP\n");
		em_game_ball_reset();
		break;

	case EM_GAME_BALL_RESET:
		APP_DBG_SIG("EM_GAME_BALL_RESET\n");
		em_game_ball_reset();
		break;

	case EM_GAME_BALL_KICK:
		APP_DBG_SIG("EM_GAME_BALL_KICK\n");
		if (get_data_len_common_msg(msg) == sizeof(em_game_ball_kick_t))
		{
			em_game_ball_start((em_game_ball_kick_t*)get_data_common_msg(msg));
		}
		break;

	case EM_GAME_BALL_UPDATE:
		if (em_game_ball.view.moving)
		{
			em_game_ball.update_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (em_game_ball.update_elapsed_ms >= EM_GAME_BALL_ANIM_TICK_INTERVAL)
			{
				em_game_ball.update_elapsed_ms -= EM_GAME_BALL_ANIM_TICK_INTERVAL;
				em_game_ball_advance();
			}
		}
		break;

	default:
		break;
	}
}
