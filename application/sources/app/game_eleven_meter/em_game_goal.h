#ifndef __EM_GAME_GOAL_H__
#define __EM_GAME_GOAL_H__

#include <stdint.h>

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"

// Bitmap dimensions and drawing offsets
#define EM_GAME_GOAL_BITMAP_WIDTH (93)
#define EM_GAME_GOAL_BITMAP_HEIGHT (38)
#define EM_GAME_GOAL_BITMAP_AXIS_X (17)
#define EM_GAME_GOAL_BITMAP_AXIS_Y (15)
#define EM_GAME_GOAL_LINE_AXIS_Y (53)

// Goal zones, results, and message contracts

typedef enum
{
	EM_GAME_GOAL_ZONE_NONE,
	EM_GAME_GOAL_ZONE_LEFT,
	EM_GAME_GOAL_ZONE_CENTER,
	EM_GAME_GOAL_ZONE_RIGHT
} em_game_goal_zone_t;

typedef enum
{
	EM_GAME_GOAL_RESULT_NONE,
	EM_GAME_GOAL_RESULT_GOAL,
	EM_GAME_GOAL_RESULT_SAVE,
	EM_GAME_GOAL_RESULT_MISS
} em_game_goal_result_t;

typedef struct
{
	uint8_t zone;
	uint8_t is_wide;
} em_game_goal_ball_t;

typedef struct
{
	uint8_t zone;
} em_game_goal_keeper_t;

typedef struct
{
	uint8_t result;
} em_game_goal_result_msg_t;

// Object state, written only by its owning task.
typedef struct
{
	em_game_goal_ball_t ball;
	em_game_goal_keeper_t keeper;
	bool ball_ready;
	bool keeper_ready;
	bool resolved;
} em_game_goal_t;

extern em_game_goal_t em_game_goal;

extern void em_game_goal_handle(ak_msg_t* msg);

#endif //__EM_GAME_GOAL_H__
