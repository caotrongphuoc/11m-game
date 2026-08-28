#include <string.h>

#include "fsm.h"
#include "message.h"
#include "port.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"

#include "task_display.h"
#include "task_list.h"

#include "screens.h"

scr_mng_t scr_mng_app;

static em_game_view_t s_em_game_view;
static em_game_ball_view_t s_em_game_ball_view;
static em_game_keeper_view_t s_em_game_keeper_view;
static em_game_shooter_view_t s_em_game_shooter_view;

static void task_display_reset_game_view()
{
	memset(&s_em_game_view, 0, sizeof(s_em_game_view));
	memset(&s_em_game_ball_view, 0, sizeof(s_em_game_ball_view));
	memset(&s_em_game_keeper_view, 0, sizeof(s_em_game_keeper_view));
	memset(&s_em_game_shooter_view, 0, sizeof(s_em_game_shooter_view));
}

static bool task_display_match_view_is_valid(const em_game_match_view_t* view)
{
	uint16_t result_count =
	    (uint16_t)view->goals + view->saves + view->misses;

	return (view->round <= EM_GAME_SCOREBOARD_TOTAL_KICKS) &&
	       (view->goals <= EM_GAME_SCOREBOARD_TOTAL_KICKS) &&
	       (view->saves <= EM_GAME_SCOREBOARD_TOTAL_KICKS) &&
	       (view->misses <= EM_GAME_SCOREBOARD_TOTAL_KICKS) &&
	       (result_count <= view->round) &&
	       (view->countdown <= 3) &&
	       (view->state <= EM_GAME_MATCH_STATE_RIP) &&
	       (view->difficulty <= EM_GAME_MATCH_DIFFICULTY_HARD) &&
	       (view->best_goals <= EM_GAME_SCOREBOARD_TOTAL_KICKS) &&
	       (view->best_difficulty <= EM_GAME_MATCH_DIFFICULTY_HARD) &&
	       (view->last_result <= EM_GAME_GOAL_RESULT_MISS) &&
	       (view->winner <= EM_GAME_SCOREBOARD_WINNER_AI);
}

static bool task_display_shooter_view_is_valid(
    const em_game_shooter_view_t* view)
{
	return (view->x >= 0) && (view->x < LCD_WIDTH) && (view->y >= 0) &&
	       (view->y < LCD_HEIGHT) &&
	       (view->frame <= EM_GAME_SHOOTER_STEP_COUNT) &&
	       (view->visible <= 1) && (view->moving <= 1) &&
	       (view->kick <= EM_GAME_SHOOTER_KICK_RIGHT);
}

static bool task_display_keeper_view_is_valid(
    const em_game_keeper_view_t* view)
{
	return (view->x >= 0) && (view->x < LCD_WIDTH) && (view->y >= 0) &&
	       (view->y < LCD_HEIGHT) &&
	       (view->frame <= EM_GAME_KEEPER_STEP_COUNT) &&
	       (view->visible <= 1) && (view->moving <= 1) &&
	       (view->dive <= EM_GAME_KEEPER_DIVE_RIGHT);
}

static bool task_display_ball_view_is_valid(const em_game_ball_view_t* view)
{
	return (view->x >= 0) && (view->x < LCD_WIDTH) && (view->y >= 0) &&
	       (view->y < LCD_HEIGHT) &&
	       (view->frame <= EM_GAME_BALL_STEP_COUNT) &&
	       (view->visible <= 1) && (view->moving <= 1) &&
	       (view->target <= EM_GAME_BALL_TARGET_WIDE_RIGHT);
}

static void task_display_update_visibility()
{
	s_em_game_view.flags = 0;

	if (s_em_game_ball_view.visible)
	{
		s_em_game_view.flags |= EM_GAME_VIEW_FLAG_BALL_VISIBLE;
	}
	if (s_em_game_keeper_view.visible)
	{
		s_em_game_view.flags |= EM_GAME_VIEW_FLAG_KEEPER_VISIBLE;
	}
	if (s_em_game_shooter_view.visible)
	{
		s_em_game_view.flags |= EM_GAME_VIEW_FLAG_SHOOTER_VISIBLE;
	}
}

const em_game_view_t* task_display_get_game_view()
{
	return &s_em_game_view;
}

void task_display(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_DISPLAY_SHOW_MENU:
	{
		APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_MENU\n");
		SCREEN_TRAN(scr_game_menu_handle, &scr_game_menu);
	}
	break;

	case EM_GAME_DISPLAY_SHOW_PENALTY:
	{
		APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_PENALTY\n");
		task_display_reset_game_view();
		SCREEN_TRAN(scr_game_penalty_handle, &scr_game_penalty);
	}
	break;

	case EM_GAME_DISPLAY_SHOW_RIP:
	{
		APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_RIP\n");
		SCREEN_TRAN(scr_game_rip_handle, &scr_game_rip);
	}
	break;

	case EM_GAME_DISPLAY_SHOW_GAME_OVER:
	{
		APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_GAME_OVER\n");
		SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
	}
	break;

	case EM_GAME_DISPLAY_UPDATE_MATCH:
	{
		if (get_data_len_common_msg(msg) != sizeof(em_game_match_view_t))
		{
			return;
		}

		em_game_match_view_t* view =
		    (em_game_match_view_t*)get_data_common_msg(msg);
		if (!task_display_match_view_is_valid(view))
		{
			return;
		}

		s_em_game_view.round = view->round;
		s_em_game_view.goals = view->goals;
		s_em_game_view.saves = view->saves;
		s_em_game_view.misses = view->misses;
		s_em_game_view.countdown = view->countdown;
		s_em_game_view.state = view->state;
		s_em_game_view.difficulty = view->difficulty;
		s_em_game_view.best_goals = view->best_goals;
		s_em_game_view.best_difficulty = view->best_difficulty;
		s_em_game_view.last_result = view->last_result;
		s_em_game_view.winner = view->winner;
	}
	break;

	case EM_GAME_DISPLAY_UPDATE_SHOOTER:
	{
		if (get_data_len_common_msg(msg) != sizeof(em_game_shooter_view_t))
		{
			return;
		}

		em_game_shooter_view_t* view =
		    (em_game_shooter_view_t*)get_data_common_msg(msg);
		if (!task_display_shooter_view_is_valid(view))
		{
			return;
		}

		memcpy(&s_em_game_shooter_view, view,
		       sizeof(s_em_game_shooter_view));
		s_em_game_view.shooter_x = s_em_game_shooter_view.x;
		s_em_game_view.shooter_y = s_em_game_shooter_view.y;
		s_em_game_view.shooter_frame = s_em_game_shooter_view.frame;
		s_em_game_view.shooter_kick = s_em_game_shooter_view.kick;
		task_display_update_visibility();
	}
	break;

	case EM_GAME_DISPLAY_UPDATE_KEEPER:
	{
		if (get_data_len_common_msg(msg) != sizeof(em_game_keeper_view_t))
		{
			return;
		}

		em_game_keeper_view_t* view =
		    (em_game_keeper_view_t*)get_data_common_msg(msg);
		if (!task_display_keeper_view_is_valid(view))
		{
			return;
		}

		memcpy(&s_em_game_keeper_view, view,
		       sizeof(s_em_game_keeper_view));
		s_em_game_view.keeper_x = s_em_game_keeper_view.x;
		s_em_game_view.keeper_y = s_em_game_keeper_view.y;
		s_em_game_view.keeper_dive = s_em_game_keeper_view.dive;
		task_display_update_visibility();
	}
	break;

	case EM_GAME_DISPLAY_UPDATE_BALL:
	{
		if (get_data_len_common_msg(msg) != sizeof(em_game_ball_view_t))
		{
			return;
		}

		em_game_ball_view_t* view =
		    (em_game_ball_view_t*)get_data_common_msg(msg);
		if (!task_display_ball_view_is_valid(view))
		{
			return;
		}

		memcpy(&s_em_game_ball_view, view,
		       sizeof(s_em_game_ball_view));
		s_em_game_view.ball_x = s_em_game_ball_view.x;
		s_em_game_view.ball_y = s_em_game_ball_view.y;
		task_display_update_visibility();
	}
	break;

	default:
		break;
	}

	scr_mng_dispatch(msg);
}
