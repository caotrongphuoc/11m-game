#ifndef __EM_GAME_GOAL_H__
#define __EM_GAME_GOAL_H__

#include <stdint.h>

#include "ak.h"
#include "message.h"

typedef enum
{
	EM_GAME_GOAL_ZONE_NONE,
	EM_GAME_GOAL_ZONE_LEFT,
	EM_GAME_GOAL_ZONE_CENTER,
	EM_GAME_GOAL_ZONE_RIGHT
} em_game_goal_zone_t;

typedef enum
{
	EM_GAME_GOAL_RESULT_NONE,
	EM_GAME_GOAL_RESULT_GOAL,
	EM_GAME_GOAL_RESULT_SAVE,
	EM_GAME_GOAL_RESULT_MISS
} em_game_goal_result_t;

typedef struct
{
	uint8_t zone;
	uint8_t is_wide;
} em_game_goal_ball_t;

static_assert(sizeof(em_game_goal_ball_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_goal_ball_t exceeds AK common message payload");

typedef struct
{
	uint8_t zone;
} em_game_goal_keeper_t;

static_assert(sizeof(em_game_goal_keeper_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_goal_keeper_t exceeds AK common message payload");

typedef struct
{
	uint8_t result;
} em_game_goal_result_msg_t;

static_assert(sizeof(em_game_goal_result_msg_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_goal_result_msg_t exceeds AK common message payload");

extern void em_game_goal_handle(ak_msg_t* msg);

#endif //__EM_GAME_GOAL_H__
