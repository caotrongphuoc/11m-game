#ifndef __EM_GAME_KEEPER_H__
#define __EM_GAME_KEEPER_H__

#include <stdbool.h>
#include <stdint.h>

#include "ak.h"
#include "message.h"

#define EM_GAME_KEEPER_START_X (64)
#define EM_GAME_KEEPER_START_Y (29)
#define EM_GAME_KEEPER_TARGET_LEFT_X (41)
#define EM_GAME_KEEPER_TARGET_CENTER_X (64)
#define EM_GAME_KEEPER_TARGET_RIGHT_X (87)
#define EM_GAME_KEEPER_TARGET_Y (29)
#define EM_GAME_KEEPER_STEP_COUNT (4)
#define EM_GAME_KEEPER_DIRECTION_COUNT (3)
#define EM_GAME_KEEPER_EASY_MATCH_FLIP_CHANCE (60)
#define EM_GAME_KEEPER_HARD_MISS_OVERRIDE_CHANCE (30)
#define EM_GAME_KEEPER_RANDOM_FALLBACK (0x6D2B79F5UL)

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
	bool visible;
	bool moving;
	uint8_t dive;
} em_game_keeper_view_t;

extern void em_game_keeper_handle(ak_msg_t* msg);

#endif //__EM_GAME_KEEPER_H__
