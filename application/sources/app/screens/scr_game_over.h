#ifndef __SCR_GAME_OVER_H__
#define __SCR_GAME_OVER_H__

#include "fsm.h"
#include "message.h"
#include "port.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"
#include "screen_manager.h"

#include "em_game_match.h"
#include "em_game_scoreboard.h"

extern view_dynamic_t dyn_view_game_over;
extern view_screen_t scr_game_over;
extern void scr_game_over_handle(ak_msg_t* msg);

#endif //__SCR_GAME_OVER_H__
