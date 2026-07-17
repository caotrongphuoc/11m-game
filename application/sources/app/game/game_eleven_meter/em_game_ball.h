#ifndef __EM_GAME_BALL_H__
#define __EM_GAME_BALL_H__

#include "ak.h"
#include "message.h"

#include "em_game_match_state.h"

#define EM_GAME_BALL_START_X (64)
#define EM_GAME_BALL_START_Y (59)
#define EM_GAME_BALL_TARGET_Y (47)
#define EM_GAME_BALL_TARGET_LEFT_X (40)
#define EM_GAME_BALL_TARGET_CENTER_X (64)
#define EM_GAME_BALL_TARGET_RIGHT_X (87)
#define EM_GAME_BALL_TARGET_WIDE_LEFT_X (20)
#define EM_GAME_BALL_TARGET_WIDE_RIGHT_X (108)
#define EM_GAME_BALL_STEP_COUNT (10)

typedef struct
{
	uint8_t direction;
	uint8_t is_wide;
} em_game_ball_kick_t;

extern em_game_ball_t em_game_ball;

extern void em_game_ball_handle(ak_msg_t* msg);

#endif //__EM_GAME_BALL_H__
