#ifndef __EM_GAME_SHOOTER_H__
#define __EM_GAME_SHOOTER_H__

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

#include "em_game_ball.h"
#include "em_game_keeper.h"

// Bitmap dimensions and drawing offsets
#define EM_GAME_SHOOTER_BITMAP_WIDTH (40)
#define EM_GAME_SHOOTER_BITMAP_HEIGHT (40)
#define EM_GAME_SHOOTER_BITMAP_BOTTOM_PADDING (6)

// Shooter geometry, animation, and accuracy tuning

#define EM_GAME_SHOOTER_START_X (44)
#define EM_GAME_SHOOTER_START_Y (63)
#define EM_GAME_SHOOTER_STEP_COUNT (4)
#define EM_GAME_SHOOTER_KICK_STEP_X (2)
#define EM_GAME_SHOOTER_MISS_BASE_CHANCE (10)
#define EM_GAME_SHOOTER_MISS_FAST_BONUS (20)
#define EM_GAME_SHOOTER_MISS_MAX_CHANCE (30)
#define EM_GAME_SHOOTER_FAST_REACTION_INTERVAL (500)
#define EM_GAME_SHOOTER_RANDOM_FALLBACK (0xA341316CUL)

// Shooter action and display snapshot

typedef enum
{
	EM_GAME_SHOOTER_KICK_NONE,
	EM_GAME_SHOOTER_KICK_LEFT,
	EM_GAME_SHOOTER_KICK_CENTER,
	EM_GAME_SHOOTER_KICK_RIGHT
} em_game_shooter_kick_t;

typedef struct
{
	int16_t x;
	int16_t y;
	uint8_t frame;
	uint8_t visible;
	uint8_t moving;
	uint8_t kick;
} em_game_shooter_view_t;

// Object state, written only by its owning task.
typedef struct
{
	em_game_shooter_view_t view;
	uint32_t random_seed;
	uint16_t selection_elapsed_ms;
	uint8_t update_elapsed_ms;
	bool kick_started;
} em_game_shooter_t;

extern em_game_shooter_t em_game_shooter;

extern void em_game_shooter_handle(ak_msg_t* msg);

#endif //__EM_GAME_SHOOTER_H__
