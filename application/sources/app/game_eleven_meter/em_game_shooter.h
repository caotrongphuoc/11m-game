#ifndef __EM_GAME_SHOOTER_H__
#define __EM_GAME_SHOOTER_H__

#include <stdbool.h>
#include <stdint.h>

#include "ak.h"
#include "message.h"

#define EM_GAME_SHOOTER_START_X (40)
#define EM_GAME_SHOOTER_START_Y (63)
#define EM_GAME_SHOOTER_STEP_COUNT (4)
#define EM_GAME_SHOOTER_MISS_BASE_CHANCE (10)
#define EM_GAME_SHOOTER_MISS_FAST_BONUS (20)
#define EM_GAME_SHOOTER_MISS_MAX_CHANCE (30)
#define EM_GAME_SHOOTER_FAST_REACTION_INTERVAL (500)
#define EM_GAME_SHOOTER_RANDOM_FALLBACK (0xA341316CUL)

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
	bool visible;
	bool moving;
	uint8_t kick;
} em_game_shooter_view_t;

static_assert(sizeof(em_game_shooter_view_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_shooter_view_t exceeds AK common message payload");

extern void em_game_shooter_handle(ak_msg_t* msg);

#endif //__EM_GAME_SHOOTER_H__
