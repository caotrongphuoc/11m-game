#include "scr_game_over.h"

/*****************************************************************************/
/* View - Game over */
/*****************************************************************************/

static void view_scr_game_over();
static void scr_game_over_winner_display(em_game_scoreboard_winner_t winner);
static void scr_game_over_score_display(uint8_t player_score, uint8_t ai_score);
static void scr_game_over_best_display(uint8_t best_goals, const char* best_difficulty_text);
static void scr_game_over_action_display();

view_dynamic_t dyn_view_game_over = {{
                                         .item_type = ITEM_TYPE_DYNAMIC,
                                     },
                                     view_scr_game_over};

view_screen_t scr_game_over = {
    &dyn_view_game_over,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

void view_scr_game_over()
{
	const em_game_view_t* view = task_display_get_game_view();
	const char* best_difficulty_text;
	uint8_t ai_score = view->saves + view->misses;

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
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(37, 2);
	view_render.print("GAME OVER");
	view_render.drawLine(18, 11, 110, 11, WHITE);

	scr_game_over_winner_display((em_game_scoreboard_winner_t)view->winner);
	scr_game_over_score_display(view->goals, ai_score);
	scr_game_over_best_display(view->best_goals, best_difficulty_text);
	scr_game_over_action_display();
}

static void scr_game_over_winner_display(em_game_scoreboard_winner_t winner)
{
	view_render.fillRect(16, 15, 96, 12, WHITE);
	view_render.setTextColor(BLACK);

	if (winner == EM_GAME_SCOREBOARD_WINNER_PLAYER)
	{
		view_render.setCursor(31, 17);
		view_render.print("PLAYER WINS!");
	}
	else if (winner == EM_GAME_SCOREBOARD_WINNER_AI)
	{
		view_render.setCursor(43, 17);
		view_render.print("AI WINS!");
	}
	else
	{
		view_render.setCursor(46, 17);
		view_render.print("NO WINNER");
	}

	view_render.setTextColor(WHITE);
}

static void scr_game_over_score_display(uint8_t player_score, uint8_t ai_score)
{
	view_render.setCursor(13, 32);
	view_render.print("YOU");
	view_render.setCursor(97, 32);
	view_render.print("CPU");

	view_render.setTextSize(2);
	view_render.setCursor(48, 28);
	view_render.print((unsigned int)player_score);
	view_render.print("-");
	view_render.print((unsigned int)ai_score);
	view_render.setTextSize(1);
}

static void scr_game_over_best_display(uint8_t best_goals, const char* best_difficulty_text)
{
	view_render.drawTriangle(8, 42, 11, 48, 5, 48, WHITE);
	view_render.drawLine(8, 48, 8, 50, WHITE);
	view_render.drawLine(5, 50, 11, 50, WHITE);

	view_render.setCursor(18, 43);
	view_render.print("BEST ");
	view_render.print((unsigned int)best_goals);
	view_render.print(" / ");
	view_render.print(best_difficulty_text);
}

static void scr_game_over_action_display()
{
	view_render.drawRect(1, 53, 75, 11, WHITE);
	view_render.setCursor(7, 55);
	view_render.print("MODE RETRY");

	view_render.fillRect(78, 53, 49, 11, WHITE);
	view_render.setTextColor(BLACK);
	view_render.setCursor(84, 55);
	view_render.print("UP HOME");
	view_render.setTextColor(WHITE);
}

/*****************************************************************************/
/* Handle - Game over */
/*****************************************************************************/

void scr_game_over_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("EM_GAME OVER SCREEN_ENTRY\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
	}
	break;

	case AC_DISPLAY_BUTTON_MODE_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_RETRY);
	}
	break;

	case AC_DISPLAY_BUTTON_UP_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_PRESSED\n");
		task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_HOME);
	}
	break;

	default:
		break;
	}
}
