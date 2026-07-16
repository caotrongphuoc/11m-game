#ifndef __EM_GAME_KEEPER_H__
#define __EM_GAME_KEEPER_H__

#include "ak.h"
#include "message.h"

#include "em_game_match_state.h"

#define EM_GAME_KEEPER_START_X (64)
#define EM_GAME_KEEPER_START_Y (25)
#define EM_GAME_KEEPER_TARGET_LEFT_X (41)
#define EM_GAME_KEEPER_TARGET_CENTER_X (64)
#define EM_GAME_KEEPER_TARGET_RIGHT_X (87)
#define EM_GAME_KEEPER_TARGET_Y (34)
#define EM_GAME_KEEPER_STEP_COUNT (4)
#define EM_GAME_KEEPER_DIRECTION_COUNT (3)
#define EM_GAME_KEEPER_EASY_MATCH_FLIP_CHANCE (60)
#define EM_GAME_KEEPER_HARD_MISS_OVERRIDE_CHANCE (30)
#define EM_GAME_KEEPER_RANDOM_FALLBACK (0x6D2B79F5UL)

typedef struct
{
	uint8_t shooter_direction;
	uint8_t difficulty;
} em_game_keeper_ai_pick_t;

extern em_game_keeper_t em_game_keeper;

extern em_game_direction_t em_game_keeper_get_direction();
extern void em_game_keeper_handle(ak_msg_t* msg);

#endif //__EM_GAME_KEEPER_H__
