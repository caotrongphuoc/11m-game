#ifndef __SCR_GAME_PENALTY_H__
#define __SCR_GAME_PENALTY_H__

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

#include "screens_bitmap.h"
#include "em_game_goal.h"
#include "em_game_ball.h"
#include "em_game_keeper.h"
#include "em_game_shooter.h"
#include "em_game_match.h"

#define SCR_GAME_PENALTY_BOX_TOP_LEFT_AXIS_X (16)
#define SCR_GAME_PENALTY_BOX_TOP_RIGHT_AXIS_X (111)
#define SCR_GAME_PENALTY_BOX_BOTTOM_LEFT_AXIS_X (0)
#define SCR_GAME_PENALTY_BOX_BOTTOM_RIGHT_AXIS_X (127)
#define SCR_GAME_PENALTY_BOX_BOTTOM_AXIS_Y (63)
#define SCR_GAME_PENALTY_HUD_SCORE_WIDTH (35)
#define SCR_GAME_PENALTY_HUD_HEIGHT (11)
#define SCR_GAME_PENALTY_HUD_RESULT_AXIS_X (68)

extern view_dynamic_t dyn_view_game_penalty;
extern view_screen_t scr_game_penalty;
extern void scr_game_penalty_handle(ak_msg_t* msg);

#endif //__SCR_GAME_PENALTY_H__
