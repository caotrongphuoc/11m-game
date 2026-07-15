#ifndef __EM_GAME_KEEPER_H__
#define __EM_GAME_KEEPER_H__

#include "ak.h"
#include "message.h"

#include "em_game_match_state.h"

#define EM_GAME_KEEPER_START_X (64)
#define EM_GAME_KEEPER_START_Y (26)
#define EM_GAME_KEEPER_TARGET_LEFT_X (41)
#define EM_GAME_KEEPER_TARGET_CENTER_X (64)
#define EM_GAME_KEEPER_TARGET_RIGHT_X (87)
#define EM_GAME_KEEPER_TARGET_Y (34)
#define EM_GAME_KEEPER_STEP_COUNT (4)
#define EM_GAME_KEEPER_RANDOM_FALLBACK (0x6D2B79F5UL)

extern em_game_keeper_t em_game_keeper;

extern em_game_direction_t em_game_keeper_get_direction();
extern void em_game_keeper_handle(ak_msg_t* msg);

#endif //__EM_GAME_KEEPER_H__
