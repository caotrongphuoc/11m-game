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

static void task_display_update_visibility() {
  s_em_game_view.flags = 0;

  if (s_em_game_ball_view.visible) {
    s_em_game_view.flags |= EM_GAME_VIEW_FLAG_BALL_VISIBLE;
  }
  if (s_em_game_keeper_view.visible) {
    s_em_game_view.flags |= EM_GAME_VIEW_FLAG_KEEPER_VISIBLE;
  }
  if (s_em_game_shooter_view.visible &&
      (!s_em_game_ball_view.moving || s_em_game_shooter_view.moving)) {
    s_em_game_view.flags |= EM_GAME_VIEW_FLAG_SHOOTER_VISIBLE;
  }
}

const em_game_view_t *task_display_get_game_view() { return &s_em_game_view; }

void task_display(ak_msg_t *msg) {
  switch (msg->sig) {
  case EM_GAME_DISPLAY_SHOW_MENU: {
    APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_MENU\n");
    SCREEN_TRAN(scr_game_menu_handle, &scr_game_menu);
  } break;

  case EM_GAME_DISPLAY_SHOW_PENALTY: {
    APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_PENALTY\n");
    SCREEN_TRAN(scr_game_penalty_handle, &scr_game_penalty);
  } break;

  case EM_GAME_DISPLAY_SHOW_RIP: {
    APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_RIP\n");
    SCREEN_TRAN(scr_game_rip_handle, &scr_game_rip);
  } break;

  case EM_GAME_DISPLAY_SHOW_GAME_OVER: {
    APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_GAME_OVER\n");
    SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
  } break;

  case EM_GAME_DISPLAY_UPDATE_MATCH: {
    if (get_data_len_common_msg(msg) != sizeof(em_game_match_view_t)) {
      return;
    }

    em_game_match_view_t *view =
        (em_game_match_view_t *)get_data_common_msg(msg);

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
  } break;

  case EM_GAME_DISPLAY_UPDATE_SHOOTER: {
    if (get_data_len_common_msg(msg) != sizeof(em_game_shooter_view_t)) {
      return;
    }

    memcpy(&s_em_game_shooter_view, get_data_common_msg(msg),
           sizeof(s_em_game_shooter_view));
    s_em_game_view.shooter_x = s_em_game_shooter_view.x;
    s_em_game_view.shooter_y = s_em_game_shooter_view.y;
    s_em_game_view.shooter_frame = s_em_game_shooter_view.frame;
    s_em_game_view.shooter_kick = s_em_game_shooter_view.kick;
    task_display_update_visibility();
  } break;

  case EM_GAME_DISPLAY_UPDATE_KEEPER: {
    if (get_data_len_common_msg(msg) != sizeof(em_game_keeper_view_t)) {
      return;
    }

    memcpy(&s_em_game_keeper_view, get_data_common_msg(msg),
           sizeof(s_em_game_keeper_view));
    s_em_game_view.keeper_x = s_em_game_keeper_view.x;
    s_em_game_view.keeper_y = s_em_game_keeper_view.y;
    s_em_game_view.keeper_dive = s_em_game_keeper_view.dive;
    task_display_update_visibility();
  } break;

  case EM_GAME_DISPLAY_UPDATE_BALL: {
    if (get_data_len_common_msg(msg) != sizeof(em_game_ball_view_t)) {
      return;
    }

    memcpy(&s_em_game_ball_view, get_data_common_msg(msg),
           sizeof(s_em_game_ball_view));
    s_em_game_view.ball_x = s_em_game_ball_view.x;
    s_em_game_view.ball_y = s_em_game_ball_view.y;
    task_display_update_visibility();
  } break;

  default:
    break;
  }

  scr_mng_dispatch(msg);
}
