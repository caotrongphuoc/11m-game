#include "scr_game_menu.h"

static void view_scr_game_menu();

view_dynamic_t dyn_view_game_menu = {{
                                         .item_type = ITEM_TYPE_DYNAMIC,
                                     },
                                     view_scr_game_menu};

view_screen_t scr_game_menu = {
    &dyn_view_game_menu,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

void view_scr_game_menu()
{
	const em_game_view_t* view = task_display_get_game_view();
	const char* difficulty_text;
	const char* best_difficulty_text;
	uint8_t difficulty_x;

	switch ((em_game_match_difficulty_t)view->difficulty)
	{
	case EM_GAME_MATCH_DIFFICULTY_EASY:
		difficulty_text = "< EASY >";
		difficulty_x = 40;
		break;

	case EM_GAME_MATCH_DIFFICULTY_HARD:
		difficulty_text = "< HARD >";
		difficulty_x = 40;
		break;

	case EM_GAME_MATCH_DIFFICULTY_NORMAL:
	default:
		difficulty_text = "< NORMAL >";
		difficulty_x = 34;
		break;
	}

	switch ((em_game_match_difficulty_t)view->best_difficulty)
	{
	case EM_GAME_MATCH_DIFFICULTY_HARD:
		best_difficulty_text = "HARD";
		break;

	case EM_GAME_MATCH_DIFFICULTY_NORMAL:
		best_difficulty_text = "NORMAL";
		break;

	case EM_GAME_MATCH_DIFFICULTY_EASY:
	default:
		best_difficulty_text = "EASY";
		break;
	}

	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);

	view_render.setCursor(16, 0);
	view_render.print("PENALTY SHOOTOUT");

	view_render.setCursor(34, 13);
	view_render.print("Difficulty:");

	view_render.setCursor(difficulty_x, 24);
	view_render.print(difficulty_text);

	view_render.setCursor(22, 36);
	view_render.print("BEST: ");
	view_render.print((unsigned int)view->best_goals);
	view_render.print(" ");
	view_render.print(best_difficulty_text);

	view_render.setCursor(22, 46);
	view_render.print("UP/DOWN: CYCLE");

	view_render.setCursor(31, 56);
	view_render.print("MODE: START");
}

void scr_game_menu_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("SCREEN_ENTRY\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_INIT);
	}
	break;

	case AC_DISPLAY_BUTON_UP_PRESSED:
	{
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_MENU_LEFT);
	}
	break;

	case AC_DISPLAY_BUTON_MODE_PRESSED:
	{
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START);
	}
	break;

	case AC_DISPLAY_BUTON_DOWN_PRESSED:
	{
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_MENU_RIGHT);
	}
	break;

	default:
		break;
	}
}
