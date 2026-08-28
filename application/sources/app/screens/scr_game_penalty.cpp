#include "scr_game_penalty.h"

/*****************************************************************************/
/* Variable declaration - Penalty screen */
/*****************************************************************************/

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
#define SCR_GAME_PENALTY_HUD_SCORE_WIDTH (35)
#define SCR_GAME_PENALTY_HUD_HEIGHT (11)

/*****************************************************************************/
/* View - Penalty screen */
/*****************************************************************************/

static void view_scr_game_penalty();
static void scr_game_penalty_hud_display(const em_game_view_t* view);
static void scr_game_penalty_stadium_display();
static void scr_game_penalty_pitch_display();
static const unsigned char*
scr_game_penalty_get_shooter_bitmap(const em_game_view_t* view);

static const unsigned char*
scr_game_penalty_get_shooter_bitmap(const em_game_view_t* view)
{
	if (view->shooter_frame == 0)
	{
		return bitmap_penalty_shooter_prepare;
	}

	if (view->shooter_frame <= 2)
	{
		return bitmap_penalty_shooter_kick;
	}

	return bitmap_penalty_shooter_follow;
}

view_dynamic_t dyn_view_game_penalty = {{
                                            .item_type = ITEM_TYPE_DYNAMIC,
                                        },
                                        view_scr_game_penalty};

view_screen_t scr_game_penalty = {
    &dyn_view_game_penalty,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

static void scr_game_penalty_frame_display(const em_game_view_t* view)
{
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	scr_game_penalty_hud_display(view);
	scr_game_penalty_stadium_display();

	view_render.drawBitmap(
	    SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_X, SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_Y,
	    bitmap_penalty_goal, SCR_GAME_PENALTY_GOAL_BITMAP_WIDTH,
	    SCR_GAME_PENALTY_GOAL_BITMAP_HEIGHT, WHITE);
	scr_game_penalty_pitch_display();
}

static void scr_game_penalty_hud_display(const em_game_view_t* view)
{
	uint8_t ai_score = view->saves + view->misses;

	view_render.fillRect(0, 0, SCR_GAME_PENALTY_HUD_SCORE_WIDTH,
	                     SCR_GAME_PENALTY_HUD_HEIGHT, WHITE);
	view_render.setTextColor(BLACK);
	view_render.setCursor(3, 2);
	view_render.print("YOU ");
	view_render.print((unsigned int)view->goals);

	view_render.fillRect(128 - SCR_GAME_PENALTY_HUD_SCORE_WIDTH, 0,
	                     SCR_GAME_PENALTY_HUD_SCORE_WIDTH,
	                     SCR_GAME_PENALTY_HUD_HEIGHT, WHITE);
	view_render.setCursor(96, 2);
	view_render.print("CPU ");
	view_render.print((unsigned int)ai_score);
	view_render.setTextColor(WHITE);

	view_render.setCursor(41, 2);
	view_render.print("R");
	view_render.print((unsigned int)view->round);
	view_render.print("/5");

	if (view->state == EM_GAME_MATCH_STATE_SHOOTER_WAIT && view->countdown > 0)
	{
		view_render.fillRect(74, 0, 11, SCR_GAME_PENALTY_HUD_HEIGHT, WHITE);
		view_render.setTextColor(BLACK);
		view_render.setCursor(77, 2);
		view_render.print((unsigned int)view->countdown);
		view_render.setTextColor(WHITE);
	}
}

static void scr_game_penalty_stadium_display()
{
	view_render.drawLine(0, 13, 127, 13, WHITE);

	for (uint8_t x = 3; x < 126; x += 8)
	{
		view_render.drawPixel(x, 11, WHITE);
	}

	view_render.drawLine(4, 16, 4, 25, WHITE);
	view_render.fillTriangle(5, 16, 5, 21, 11, 18, WHITE);
	view_render.drawLine(123, 16, 123, 25, WHITE);
	view_render.fillTriangle(122, 16, 122, 21, 116, 18, WHITE);
}

static void scr_game_penalty_pitch_display()
{

	view_render.drawLine(0, SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y, 127,
	                     SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y, WHITE);
	view_render.drawLine(SCR_GAME_PENALTY_BOX_TOP_LEFT_AXIS_X,
	                     SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_LEFT_AXIS_X,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y, WHITE);
	view_render.drawLine(SCR_GAME_PENALTY_BOX_TOP_RIGHT_AXIS_X,
	                     SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_RIGHT_AXIS_X,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y, WHITE);
	view_render.drawLine(SCR_GAME_PENALTY_BOX_BOTTOM_LEFT_AXIS_X,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_RIGHT_AXIS_X,
	                     SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y, WHITE);

	view_render.drawPixel(63, 59, WHITE);
	view_render.drawPixel(64, 59, WHITE);
	view_render.drawPixel(65, 59, WHITE);
}

static void scr_game_penalty_keeper_display(const em_game_view_t* view)
{
	if ((view->flags & EM_GAME_VIEW_FLAG_KEEPER_VISIBLE) == 0)
	{
		return;
	}

	if (view->keeper_dive == EM_GAME_KEEPER_DIVE_LEFT)
	{
		view_render.drawBitmap(
		    view->keeper_x - (SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH / 2),
		    view->keeper_y + SCR_GAME_PENALTY_KEEPER_DIVE_LEFT_Y_OFFSET,
		    bitmap_penalty_keeper_dive_left,
		    SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH,
		    SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_HEIGHT, WHITE);
	}
	else if (view->keeper_dive == EM_GAME_KEEPER_DIVE_RIGHT)
	{
		view_render.drawBitmap(
		    view->keeper_x - (SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH / 2),
		    view->keeper_y + SCR_GAME_PENALTY_KEEPER_DIVE_RIGHT_Y_OFFSET,
		    bitmap_penalty_keeper_dive_right,
		    SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH,
		    SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_HEIGHT, WHITE);
	}
	else
	{
		view_render.drawBitmap(
		    view->keeper_x - (SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_WIDTH / 2),
		    view->keeper_y + SCR_GAME_PENALTY_KEEPER_IDLE_Y_OFFSET,
		    bitmap_penalty_keeper_idle, SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_WIDTH,
		    SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_HEIGHT, WHITE);
	}
}

static void scr_game_penalty_shooter_display(const em_game_view_t* view)
{
	if ((view->flags & EM_GAME_VIEW_FLAG_SHOOTER_VISIBLE) == 0)
	{
		return;
	}

	view_render.drawBitmap(
	    view->shooter_x - (SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH / 2),
	    view->shooter_y - SCR_GAME_PENALTY_SHOOTER_BITMAP_HEIGHT +
	        SCR_GAME_PENALTY_SHOOTER_BITMAP_BOTTOM_PADDING,
	    scr_game_penalty_get_shooter_bitmap(view),
	    SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH,
	    SCR_GAME_PENALTY_SHOOTER_BITMAP_HEIGHT, WHITE);
}

static void scr_game_penalty_ball_display(const em_game_view_t* view)
{
	if ((view->flags & EM_GAME_VIEW_FLAG_BALL_VISIBLE) == 0)
	{
		return;
	}

	view_render.drawBitmap(
	    view->ball_x - (SCR_GAME_PENALTY_BALL_BITMAP_WIDTH / 2),
	    view->ball_y - (SCR_GAME_PENALTY_BALL_BITMAP_HEIGHT / 2),
	    bitmap_penalty_ball, SCR_GAME_PENALTY_BALL_BITMAP_WIDTH,
	    SCR_GAME_PENALTY_BALL_BITMAP_HEIGHT, WHITE);
}

void view_scr_game_penalty()
{
	const em_game_view_t* view = task_display_get_game_view();

	view_render.clear();
	scr_game_penalty_frame_display(view);
	scr_game_penalty_keeper_display(view);
	scr_game_penalty_shooter_display(view);
	scr_game_penalty_ball_display(view);
}

/*****************************************************************************/
/* Handle - Penalty screen */
/*****************************************************************************/

void scr_game_penalty_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("EM_GAME PENALTY SCREEN_ENTRY\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
		task_post_pure_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_SETUP);
		task_post_pure_msg(EM_GAME_BALL_ID, EM_GAME_BALL_SETUP);
		task_post_pure_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_SETUP);
		task_post_pure_msg(EM_GAME_GOAL_ID, EM_GAME_GOAL_SETUP);
		timer_remove_attr(AC_TASK_DISPLAY_ID, EM_GAME_TIME_TICK);
		timer_set(AC_TASK_DISPLAY_ID, EM_GAME_TIME_TICK,
		          EM_GAME_TIME_TICK_INTERVAL, TIMER_PERIODIC);
	}
	break;

	case EM_GAME_TIME_TICK:
	{
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_UPDATE);
		task_post_pure_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_UPDATE);
		task_post_pure_msg(EM_GAME_BALL_ID, EM_GAME_BALL_UPDATE);
		task_post_pure_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_UPDATE);
	}
	break;

	case AC_DISPLAY_BUTTON_UP_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_KICK_LEFT);
	}
	break;

	case AC_DISPLAY_BUTTON_MODE_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_KICK_CENTER);
	}
	break;

	case AC_DISPLAY_BUTTON_DOWN_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_KICK_RIGHT);
	}
	break;

	default:
		break;
	}
}
