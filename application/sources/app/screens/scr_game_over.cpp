#include "scr_game_over.h"

static void view_scr_game_over();

view_dynamic_t dyn_view_game_over = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_over
};

view_screen_t scr_game_over = {
	&dyn_view_game_over,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_game_over()
{
	view_render.clear();
	view_render.setTextColor(WHITE);

	view_render.setTextSize(2);
	view_render.setCursor(16, 2);
	view_render.print("GAME OVER");

	view_render.setTextSize(1);
	view_render.setCursor(34, 30);
	view_render.print("WINNER: --");

	view_render.setCursor(7, 55);
	view_render.print("MODE:RETRY  UP:HOME");
}

void scr_game_over_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("SCREEN_ENTRY\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
	}
	break;

	case AC_DISPLAY_BUTON_MODE_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_RETRY);
	}
	break;

	case AC_DISPLAY_BUTON_UP_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTON_UP_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_HOME);
	}
	break;

	default:
		break;
	}
}
