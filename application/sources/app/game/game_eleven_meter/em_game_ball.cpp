#include "ak.h"
#include "message.h"
#include "port.h"
#include "timer.h"

#include "app.h"
#include "task_list.h"

#include "em_game_ball.h"
#include "em_game_goal.h"

static em_game_ball_view_t s_ball;
static int16_t s_target_x;

static void em_game_ball_publish_view()
{
	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_BALL,
	                     (uint8_t*)&s_ball, sizeof(s_ball));
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
	timer_remove_attr(EM_GAME_BALL_ID, EM_GAME_BALL_ANIM_TICK);

	s_ball.x = EM_GAME_BALL_START_X;
	s_ball.y = EM_GAME_BALL_START_Y;
	s_ball.frame = 0;
	s_ball.visible = true;
	s_ball.moving = false;
	s_ball.target = EM_GAME_BALL_TARGET_NONE;
	s_target_x = EM_GAME_BALL_START_X;

	em_game_ball_publish_view();
}

static void em_game_ball_start(const em_game_ball_kick_t* kick)
{
	em_game_ball_target_t target = (em_game_ball_target_t)kick->target;

	if ((target < EM_GAME_BALL_TARGET_LEFT) ||
	    (target > EM_GAME_BALL_TARGET_WIDE_RIGHT))
	{
		return;
	}

	s_ball.x = EM_GAME_BALL_START_X;
	s_ball.y = EM_GAME_BALL_START_Y;
	s_ball.frame = 0;
	s_ball.visible = true;
	s_ball.moving = true;
	s_ball.target = (uint8_t)target;
	s_target_x = em_game_ball_get_target_x(target);

	timer_set(EM_GAME_BALL_ID, EM_GAME_BALL_ANIM_TICK,
	          EM_GAME_BALL_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
	em_game_ball_publish_view();
}

static void em_game_ball_report_arrival()
{
	em_game_goal_ball_t ball;

	switch ((em_game_ball_target_t)s_ball.target)
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
	if (!s_ball.moving)
	{
		return;
	}

	s_ball.frame++;
	s_ball.x = EM_GAME_BALL_START_X +
	           ((s_target_x - EM_GAME_BALL_START_X) * s_ball.frame) /
	               EM_GAME_BALL_STEP_COUNT;
	s_ball.y = EM_GAME_BALL_START_Y +
	           ((EM_GAME_BALL_TARGET_Y - EM_GAME_BALL_START_Y) * s_ball.frame) /
	               EM_GAME_BALL_STEP_COUNT;

	if (s_ball.frame >= EM_GAME_BALL_STEP_COUNT)
	{
		s_ball.x = s_target_x;
		s_ball.y = EM_GAME_BALL_TARGET_Y;
		s_ball.moving = false;
		timer_remove_attr(EM_GAME_BALL_ID, EM_GAME_BALL_ANIM_TICK);
		em_game_ball_report_arrival();
	}

	em_game_ball_publish_view();
}

void em_game_ball_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_BALL_RESET:
		em_game_ball_reset();
		break;

	case EM_GAME_BALL_KICK:
		if (get_data_len_common_msg(msg) == sizeof(em_game_ball_kick_t))
		{
			em_game_ball_start((em_game_ball_kick_t*)get_data_common_msg(msg));
		}
		break;

	case EM_GAME_BALL_ANIM_TICK:
		em_game_ball_advance();
		break;

	default:
		break;
	}
}
