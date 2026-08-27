#include "scr_game_menu.h"

/*****************************************************************************/
/* View - Game menu */
/*****************************************************************************/

static void view_scr_game_menu();
static void scr_game_menu_title_display();
static void scr_game_menu_difficulty_display(em_game_match_difficulty_t difficulty);
static void scr_game_menu_difficulty_option_display(uint8_t x, uint8_t width, const char* text, bool selected);
static void scr_game_menu_best_display(uint8_t best_goals, const char* best_difficulty_text);
static void scr_game_menu_start_display();

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
	const char* best_difficulty_text;

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

	scr_game_menu_title_display();
	scr_game_menu_difficulty_display((em_game_match_difficulty_t)view->difficulty);
	scr_game_menu_best_display(view->best_goals, best_difficulty_text);
	scr_game_menu_start_display();
}

static void scr_game_menu_title_display()
{
	view_render.drawCircle(7, 7, 5, WHITE);
	view_render.drawLine(4, 4, 10, 10, WHITE);
	view_render.drawLine(10, 4, 4, 10, WHITE);

	view_render.setCursor(25, 3);
	view_render.print("ELEVEN METER");

	view_render.drawRect(113, 2, 13, 10, WHITE);
	view_render.drawLine(113, 2, 119, 7, WHITE);
	view_render.drawLine(126, 2, 120, 7, WHITE);
	view_render.drawLine(119, 7, 113, 11, WHITE);
	view_render.drawLine(120, 7, 126, 11, WHITE);
}

static void scr_game_menu_difficulty_display(em_game_match_difficulty_t difficulty)
{
	view_render.setCursor(37, 15);
	view_render.print("DIFFICULTY");

	scr_game_menu_difficulty_option_display(1, 36, "EASY", difficulty == EM_GAME_MATCH_DIFFICULTY_EASY);
	scr_game_menu_difficulty_option_display(39, 50, "NORMAL", difficulty == EM_GAME_MATCH_DIFFICULTY_NORMAL);
	scr_game_menu_difficulty_option_display(91, 36, "HARD", difficulty == EM_GAME_MATCH_DIFFICULTY_HARD);
}

static void scr_game_menu_difficulty_option_display(uint8_t x, uint8_t width, const char* text, bool selected)
{
	uint8_t text_x = x + ((width - (strlen(text) * 6)) / 2);

	if (selected)
	{
		view_render.fillRect(x, 24, width, 12, WHITE);
		view_render.setTextColor(BLACK);
	}
	else
	{
		view_render.drawRect(x, 24, width, 12, WHITE);
		view_render.setTextColor(WHITE);
	}

	view_render.setCursor(text_x, 26);
	view_render.print(text);
	view_render.setTextColor(WHITE);
}

static void scr_game_menu_best_display(uint8_t best_goals, const char* best_difficulty_text)
{
	view_render.drawTriangle(8, 40, 11, 46, 5, 46, WHITE);
	view_render.drawLine(8, 46, 8, 49, WHITE);
	view_render.drawLine(5, 49, 11, 49, WHITE);

	view_render.setCursor(18, 41);
	view_render.print("BEST ");
	view_render.print((unsigned int)best_goals);
	view_render.print(" / ");
	view_render.print(best_difficulty_text);

	view_render.fillTriangle(118, 40, 114, 44, 122, 44, WHITE);
	view_render.fillTriangle(118, 49, 114, 45, 122, 45, WHITE);
}

static void scr_game_menu_start_display()
{
	view_render.fillRect(18, 52, 92, 12, WHITE);
	view_render.fillTriangle(27, 55, 27, 61, 33, 58, BLACK);
	view_render.setTextColor(BLACK);
	view_render.setCursor(42, 54);
	view_render.print("MODE  START");
	view_render.setTextColor(WHITE);
}

/*****************************************************************************/
/* Handle - Game menu */
/*****************************************************************************/

void scr_game_menu_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("EM_GAME MENU SCREEN_ENTRY\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_INIT);
	}
	break;

	case AC_DISPLAY_BUTTON_UP_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_MENU_LEFT);
	}
	break;

	case AC_DISPLAY_BUTTON_MODE_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START);
	}
	break;

	case AC_DISPLAY_BUTTON_DOWN_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_MENU_RIGHT);
	}
	break;

	default:
		break;
	}
}
