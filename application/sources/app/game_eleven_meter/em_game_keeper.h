#ifndef __EM_GAME_KEEPER_H__
#define __EM_GAME_KEEPER_H__

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

#include "sys_ctrl.h"

#include "em_game_goal.h"

// Bitmap dimensions and drawing offsets
#define EM_GAME_KEEPER_IDLE_BITMAP_WIDTH (40)
#define EM_GAME_KEEPER_IDLE_BITMAP_HEIGHT (40)
#define EM_GAME_KEEPER_IDLE_Y_OFFSET (-8)
#define EM_GAME_KEEPER_DIVE_BITMAP_WIDTH (40)
#define EM_GAME_KEEPER_DIVE_BITMAP_HEIGHT (40)
#define EM_GAME_KEEPER_DIVE_LEFT_Y_OFFSET (-6)
#define EM_GAME_KEEPER_DIVE_RIGHT_Y_OFFSET (-8)

// Keeper geometry, animation, and difficulty tuning

#define EM_GAME_KEEPER_START_X (64)
#define EM_GAME_KEEPER_START_Y (29)
#define EM_GAME_KEEPER_TARGET_LEFT_X (41)
#define EM_GAME_KEEPER_TARGET_CENTER_X (64)
#define EM_GAME_KEEPER_TARGET_RIGHT_X (87)
#define EM_GAME_KEEPER_TARGET_Y (29)
#define EM_GAME_KEEPER_STEP_COUNT (6)
#define EM_GAME_KEEPER_DIRECTION_COUNT (3)
#define EM_GAME_KEEPER_EASY_MATCH_FLIP_CHANCE (60)
#define EM_GAME_KEEPER_HARD_MISS_OVERRIDE_CHANCE (30)
#define EM_GAME_KEEPER_RANDOM_FALLBACK (0x6D2B79F5UL)

// Keeper message contracts and display snapshot

typedef enum
{
	EM_GAME_KEEPER_DIVE_NONE,
	EM_GAME_KEEPER_DIVE_LEFT,
	EM_GAME_KEEPER_DIVE_CENTER,
	EM_GAME_KEEPER_DIVE_RIGHT
} em_game_keeper_dive_t;

typedef enum
{
	EM_GAME_KEEPER_DIFFICULTY_EASY,
	EM_GAME_KEEPER_DIFFICULTY_NORMAL,
	EM_GAME_KEEPER_DIFFICULTY_HARD
} em_game_keeper_difficulty_t;

typedef struct
{
	uint8_t difficulty;
} em_game_keeper_difficulty_msg_t;

typedef enum
{
	EM_GAME_KEEPER_SHOT_NONE,
	EM_GAME_KEEPER_SHOT_LEFT,
	EM_GAME_KEEPER_SHOT_CENTER,
	EM_GAME_KEEPER_SHOT_RIGHT
} em_game_keeper_shot_t;

typedef struct
{
	uint8_t shot_zone;
} em_game_keeper_react_t;

typedef struct
{
	int16_t x;
	int16_t y;
	uint8_t frame;
	uint8_t visible;
	uint8_t moving;
	uint8_t dive;
} em_game_keeper_view_t;

// Object state, written only by its owning task.
typedef struct
{
	em_game_keeper_view_t view;
	em_game_keeper_difficulty_t difficulty;
	int16_t target_x;
	uint32_t random_seed;
	uint8_t update_elapsed_ms;
	bool reaction_started;
} em_game_keeper_t;

extern em_game_keeper_t em_game_keeper;

extern void em_game_keeper_handle(ak_msg_t* msg);

#endif //__EM_GAME_KEEPER_H__
