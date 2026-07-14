#include "scr_game_penalty.h"

static void view_scr_game_penalty();

view_dynamic_t dyn_view_game_penalty = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_penalty
};

view_screen_t scr_game_penalty = {
	&dyn_view_game_penalty,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_game_penalty()
{
	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);

	view_render.setCursor(0, 0);
	view_render.print("P: 0  A: 0");

	view_render.setCursor(76, 0);
	view_render.print("ROUND 0/5");

	view_render.setCursor(61, 16);
	view_render.print("3");

	view_render.drawRect(29, 24, 70, 30, WHITE);
	view_render.drawLine(52, 24, 52, 53, WHITE);
	view_render.drawLine(75, 24, 75, 53, WHITE);

	view_render.setCursor(4, 56);
	view_render.print("UP:L  MODE:C  DOWN:R");
}

void scr_game_penalty_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("SCREEN_ENTRY\n");
	}
	break;

	default:
		break;
	}
}
