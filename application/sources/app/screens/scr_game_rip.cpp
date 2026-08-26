#include "scr_game_rip.h"

static void view_scr_game_rip();

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

	view_render.setTextSize(2);
	view_render.setCursor(4, 4);
	view_render.print("MATCH OVER");

	view_render.setTextSize(1);
	view_render.setCursor(49, 31);
	view_render.print(". . .");

	view_render.setCursor(22, 55);
	view_render.print("MODE: continue");
}

void scr_game_rip_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("SCREEN_ENTRY\n");
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
