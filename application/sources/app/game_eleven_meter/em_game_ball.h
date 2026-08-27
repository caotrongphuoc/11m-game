#ifndef __EM_GAME_BALL_H__
#define __EM_GAME_BALL_H__

#include <stdbool.h>
#include <stdint.h>

#include "ak.h"
#include "message.h"

/*****************************************************************************/
/* Ball geometry and animation */
/*****************************************************************************/

#define EM_GAME_BALL_START_X (64)
#define EM_GAME_BALL_START_Y (59)
#define EM_GAME_BALL_TARGET_Y (47)
#define EM_GAME_BALL_TARGET_LEFT_X (40)
#define EM_GAME_BALL_TARGET_CENTER_X (64)
#define EM_GAME_BALL_TARGET_RIGHT_X (87)
#define EM_GAME_BALL_TARGET_WIDE_LEFT_X (20)
#define EM_GAME_BALL_TARGET_WIDE_RIGHT_X (108)
#define EM_GAME_BALL_STEP_COUNT (8)

/*****************************************************************************/
/* Ball message contracts and display snapshot */
/*****************************************************************************/

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

static_assert(sizeof(em_game_ball_kick_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_ball_kick_t exceeds AK common message payload");

typedef struct
{
	int16_t x;
	int16_t y;
	uint8_t frame;
	bool visible;
	bool moving;
	uint8_t target;
} em_game_ball_view_t;

static_assert(sizeof(em_game_ball_view_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_ball_view_t exceeds AK common message payload");

extern void em_game_ball_handle(ak_msg_t* msg);

#endif //__EM_GAME_BALL_H__
