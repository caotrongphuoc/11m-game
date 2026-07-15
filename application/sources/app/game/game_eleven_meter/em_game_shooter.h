#ifndef __EM_GAME_SHOOTER_H__
#define __EM_GAME_SHOOTER_H__

#include "em_game_match_state.h"

#define EM_GAME_SHOOTER_START_X (64)
#define EM_GAME_SHOOTER_START_Y (58)
#define EM_GAME_SHOOTER_STEP_COUNT (4)

extern em_game_shooter_t em_game_shooter;

extern void em_game_shooter_handle(ak_msg_t* msg);

#endif //__EM_GAME_SHOOTER_H__
