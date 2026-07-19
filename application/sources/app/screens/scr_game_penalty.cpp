#include "scr_game_penalty.h"

#include "em_game_keeper.h"
#include "em_game_shooter.h"

#define SCR_GAME_PENALTY_BALL_BITMAP_WIDTH (7)
#define SCR_GAME_PENALTY_BALL_BITMAP_HEIGHT (7)
#define SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_WIDTH (40)
#define SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_HEIGHT (40)
#define SCR_GAME_PENALTY_KEEPER_IDLE_Y_OFFSET (-8)
#define SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH (40)
#define SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_HEIGHT (40)
#define SCR_GAME_PENALTY_KEEPER_DIVE_LEFT_Y_OFFSET (-6)
#define SCR_GAME_PENALTY_KEEPER_DIVE_RIGHT_Y_OFFSET (-8)
#define SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH (40)
#define SCR_GAME_PENALTY_SHOOTER_BITMAP_HEIGHT (40)
#define SCR_GAME_PENALTY_SHOOTER_BITMAP_BOTTOM_PADDING (6)
#define SCR_GAME_PENALTY_GOAL_BITMAP_WIDTH (93)
#define SCR_GAME_PENALTY_GOAL_BITMAP_HEIGHT (38)
#define SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_X (17)
#define SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_Y (15)
#define SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y (53)
#define SCR_GAME_PENALTY_BOX_TOP_LEFT_AXIS_X (16)
#define SCR_GAME_PENALTY_BOX_TOP_RIGHT_AXIS_X (111)
#define SCR_GAME_PENALTY_BOX_BOTTOM_LEFT_AXIS_X (0)
#define SCR_GAME_PENALTY_BOX_BOTTOM_RIGHT_AXIS_X (127)
#define SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y (63)
#define SCR_GAME_PENALTY_COUNTDOWN_TEXT_SIZE (2)
#define SCR_GAME_PENALTY_COUNTDOWN_AXIS_X (58)
#define SCR_GAME_PENALTY_COUNTDOWN_AXIS_Y (7)

static void view_scr_game_penalty();
static const unsigned char *scr_game_penalty_get_shooter_bitmap();

static const unsigned char *scr_game_penalty_get_shooter_bitmap()
{
	if (em_game_shooter.frame == 0)
	{
		return bitmap_penalty_shooter_prepare;
	}

	if (em_game_shooter.frame <= 2)
	{
		return bitmap_penalty_shooter_kick;
	}

	return bitmap_penalty_shooter_follow;
}

view_dynamic_t dyn_view_game_penalty = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_penalty};

view_screen_t scr_game_penalty = {
	&dyn_view_game_penalty,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_game_penalty()
{
	const em_game_view_t *view = task_display_get_game_view();
	uint8_t ai_score = view->saves + view->misses;

	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);

	view_render.setCursor(0, 0);
	view_render.print("P:");
	view_render.print((unsigned int)view->goals);
	view_render.print(" A:");
	view_render.print((unsigned int)ai_score);

	view_render.setCursor(70, 0);
	view_render.print("ROUND ");
	view_render.print((unsigned int)view->round);
	view_render.print("/5");

	if (view->state == EM_GAME_STATE_SHOOTER_WAIT &&
		view->countdown > 0)
	{
		view_render.setTextSize(SCR_GAME_PENALTY_COUNTDOWN_TEXT_SIZE);
		view_render.setCursor(SCR_GAME_PENALTY_COUNTDOWN_AXIS_X,
							  SCR_GAME_PENALTY_COUNTDOWN_AXIS_Y);
		view_render.print((unsigned int)view->countdown);
	}

	view_render.drawBitmap(
		SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_X,
		SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_Y,
		bitmap_penalty_goal,
		SCR_GAME_PENALTY_GOAL_BITMAP_WIDTH,
		SCR_GAME_PENALTY_GOAL_BITMAP_HEIGHT,
		WHITE);

	view_render.drawLine(0, SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y, 127, SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y, WHITE);
	view_render.drawLine(SCR_GAME_PENALTY_BOX_TOP_LEFT_AXIS_X, SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_LEFT_AXIS_X, SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y, WHITE);
	view_render.drawLine(SCR_GAME_PENALTY_BOX_TOP_RIGHT_AXIS_X, SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_RIGHT_AXIS_X, SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y, WHITE);
	view_render.drawLine(SCR_GAME_PENALTY_BOX_BOTTOM_LEFT_AXIS_X, SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_RIGHT_AXIS_X, SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y, WHITE);

	if ((view->flags & EM_GAME_VIEW_FLAG_KEEPER_VISIBLE) != 0)
	{
		if (em_game_keeper.direction == EM_GAME_DIRECTION_LEFT)
		{
			view_render.drawBitmap(
				view->keeper_x - (SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH / 2),
				view->keeper_y + SCR_GAME_PENALTY_KEEPER_DIVE_LEFT_Y_OFFSET,
				bitmap_penalty_keeper_dive_left,
				SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH,
				SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_HEIGHT,
				WHITE);
		}
		else if (em_game_keeper.direction == EM_GAME_DIRECTION_RIGHT)
		{
			view_render.drawBitmap(
				view->keeper_x - (SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH / 2),
				view->keeper_y + SCR_GAME_PENALTY_KEEPER_DIVE_RIGHT_Y_OFFSET,
				bitmap_penalty_keeper_dive_right,
				SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH,
				SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_HEIGHT,
				WHITE);
		}
		else
		{
			view_render.drawBitmap(
				view->keeper_x - (SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_WIDTH / 2),
				view->keeper_y + SCR_GAME_PENALTY_KEEPER_IDLE_Y_OFFSET,
				bitmap_penalty_keeper_idle,
				SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_WIDTH,
				SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_HEIGHT,
				WHITE);
		}
	}

	if ((view->flags & EM_GAME_VIEW_FLAG_SHOOTER_VISIBLE) != 0)
	{
		view_render.drawBitmap(
			view->shooter_x - (SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH / 2),
			view->shooter_y - SCR_GAME_PENALTY_SHOOTER_BITMAP_HEIGHT + SCR_GAME_PENALTY_SHOOTER_BITMAP_BOTTOM_PADDING,
			scr_game_penalty_get_shooter_bitmap(),
			SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH,
			SCR_GAME_PENALTY_SHOOTER_BITMAP_HEIGHT,
			WHITE);
	}

	if ((view->flags & EM_GAME_VIEW_FLAG_BALL_VISIBLE) != 0)
	{
		view_render.drawBitmap(
			view->ball_x - (SCR_GAME_PENALTY_BALL_BITMAP_WIDTH / 2),
			view->ball_y - (SCR_GAME_PENALTY_BALL_BITMAP_HEIGHT / 2),
			bitmap_penalty_ball,
			SCR_GAME_PENALTY_BALL_BITMAP_WIDTH,
			SCR_GAME_PENALTY_BALL_BITMAP_HEIGHT,
			WHITE);
	}
}

void scr_game_penalty_handle(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("SCREEN_ENTRY\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
	}
	break;

	case AC_DISPLAY_BUTON_UP_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_UP_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_KICK_LEFT);
	}
	break;

	case AC_DISPLAY_BUTON_MODE_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_KICK_CENTER);
	}
	break;

	case AC_DISPLAY_BUTON_DOWN_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_DOWN_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_KICK_RIGHT);
	}
	break;

	default:
		break;
	}
}
