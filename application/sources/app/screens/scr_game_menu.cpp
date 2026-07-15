#include "scr_game_menu.h"

static void view_scr_game_menu();

view_dynamic_t dyn_view_game_menu = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_menu
};

view_screen_t scr_game_menu = {
	&dyn_view_game_menu,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_game_menu()
{
	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);

	view_render.setCursor(16, 2);
	view_render.print("PENALTY SHOOTOUT");

	view_render.setCursor(7, 27);
	view_render.print("PRESS MODE TO START");

	view_render.setCursor(28, 55);
	view_render.print("MODE = START");
}

void scr_game_menu_handle(ak_msg_t* msg)
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
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START);
		SCREEN_TRAN(scr_game_penalty_handle, &scr_game_penalty);
	}
	break;

	default:
		break;
	}
}
