#include "scr_game_rip.h"

/*****************************************************************************/
/* View - Game RIP */
/*****************************************************************************/

static void view_scr_game_rip();
static void scr_game_rip_goal_display();
static void scr_game_rip_continue_display();

view_dynamic_t dyn_view_game_rip = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_game_rip};

view_screen_t scr_game_rip = {
    &dyn_view_game_rip,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

void view_scr_game_rip()
{
	view_render.clear();
	view_render.setTextColor(WHITE);

	view_render.fillRect(0, 0, 128, 17, WHITE);
	view_render.setTextColor(BLACK);
	view_render.setTextSize(1);
	view_render.setCursor(34, 5);
	view_render.print("MATCH OVER");
	view_render.setTextColor(WHITE);

	scr_game_rip_goal_display();
	scr_game_rip_continue_display();
}

static void scr_game_rip_goal_display()
{
	view_render.drawRect(33, 22, 62, 23, WHITE);
	view_render.drawLine(33, 22, 43, 31, WHITE);
	view_render.drawLine(95, 22, 85, 31, WHITE);
	view_render.drawLine(43, 31, 85, 31, WHITE);
	view_render.drawLine(43, 31, 33, 44, WHITE);
	view_render.drawLine(85, 31, 95, 44, WHITE);

	view_render.drawCircle(64, 38, 5, WHITE);
	view_render.drawLine(61, 35, 67, 41, WHITE);
	view_render.drawLine(67, 35, 61, 41, WHITE);
}

static void scr_game_rip_continue_display()
{
	view_render.fillRect(22, 51, 84, 13, WHITE);
	view_render.fillTriangle(29, 54, 29, 61, 36, 57, BLACK);
	view_render.setTextColor(BLACK);
	view_render.setCursor(42, 54);
	view_render.print("CONTINUE");
	view_render.setTextColor(WHITE);
}

/*****************************************************************************/
/* Handle - Game RIP */
/*****************************************************************************/

void scr_game_rip_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("EM_GAME RIP SCREEN_ENTRY\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
	}
	break;

	case AC_DISPLAY_BUTTON_MODE_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SKIP_RIP);
	}
	break;

	default:
		break;
	}
}
