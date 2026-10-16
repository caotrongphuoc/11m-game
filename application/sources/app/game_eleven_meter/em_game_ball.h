#ifndef __EM_GAME_BALL_H__
#define __EM_GAME_BALL_H__

#include <stdbool.h>
#include <stdint.h>

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"

#include "em_game_goal.h"

// Bitmap dimensions and drawing offsets
#define EM_GAME_BALL_BITMAP_WIDTH (7)
#define EM_GAME_BALL_BITMAP_HEIGHT (7)

// Ball geometry and animation

#define EM_GAME_BALL_START_X (64)
#define EM_GAME_BALL_START_Y (59)
#define EM_GAME_BALL_TARGET_Y (47)
#define EM_GAME_BALL_TARGET_LEFT_X (40)
#define EM_GAME_BALL_TARGET_CENTER_X (64)
#define EM_GAME_BALL_TARGET_RIGHT_X (87)
#define EM_GAME_BALL_TARGET_WIDE_LEFT_X (20)
#define EM_GAME_BALL_TARGET_WIDE_RIGHT_X (108)
#define EM_GAME_BALL_STEP_COUNT (8)

// Ball message contracts and display snapshot

typedef enum
{
	EM_GAME_BALL_TARGET_NONE,
	EM_GAME_BALL_TARGET_LEFT,
	EM_GAME_BALL_TARGET_CENTER,
	EM_GAME_BALL_TARGET_RIGHT,
	EM_GAME_BALL_TARGET_WIDE_LEFT,
	EM_GAME_BALL_TARGET_WIDE_RIGHT
} em_game_ball_target_t;

typedef struct
{
	uint8_t target;
} em_game_ball_kick_t;

typedef struct
{
	int16_t x;
	int16_t y;
	uint8_t frame;
	uint8_t visible;
	uint8_t moving;
	uint8_t target;
} em_game_ball_view_t;

// Object state, written only by its owning task.
typedef struct
{
	em_game_ball_view_t view;
	int16_t target_x;
	uint8_t update_elapsed_ms;
	bool kick_started;
} em_game_ball_t;

extern em_game_ball_t em_game_ball;

extern void em_game_ball_handle(ak_msg_t* msg);

#endif //__EM_GAME_BALL_H__
