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

#define SCR_GAME_PENALTY_BALL_BITMAP_WIDTH (7)
#define SCR_GAME_PENALTY_BALL_BITMAP_HEIGHT (7)
#define SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_WIDTH (40)
#define SCR_GAME_PENALTY_KEEPER_IDLE_BITMAP_HEIGHT (40)
#define SCR_GAME_PENALTY_KEEPER_IDLE_Y_OFFSET (-8)
#define SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH (40)
#define SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_HEIGHT (40)
#define SCR_GAME_PENALTY_KEEPER_DIVE_LEFT_Y_OFFSET (-6)
#define SCR_GAME_PENALTY_KEEPER_DIVE_RIGHT_Y_OFFSET (-8)
#define SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH (40)
#define SCR_GAME_PENALTY_SHOOTER_BITMAP_HEIGHT (40)
#define SCR_GAME_PENALTY_SHOOTER_BITMAP_BOTTOM_PADDING (6)
#define SCR_GAME_PENALTY_GOAL_BITMAP_WIDTH (93)
#define SCR_GAME_PENALTY_GOAL_BITMAP_HEIGHT (38)
#define SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_X (17)
#define SCR_GAME_PENALTY_GOAL_BITMAP_AXIS_Y (15)
#define SCR_GAME_PENALTY_GOAL_LINE_AXIS_Y (53)
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
