#ifndef __EM_GAME_MATCH_H__
#define __EM_GAME_MATCH_H__

#include <stdint.h>

#include "ak.h"
#include "message.h"

/*****************************************************************************/
/* Match state and difficulty */
/*****************************************************************************/

typedef enum
{
	EM_GAME_MATCH_STATE_MENU,
	EM_GAME_MATCH_STATE_ROUND_START,
	EM_GAME_MATCH_STATE_SHOOTER_WAIT,
	EM_GAME_MATCH_STATE_REVEAL,
	EM_GAME_MATCH_STATE_ROUND_END,
	EM_GAME_MATCH_STATE_GAME_OVER,
	EM_GAME_MATCH_STATE_RIP
} em_game_match_state_t;

typedef enum
{
	EM_GAME_MATCH_DIFFICULTY_EASY,
	EM_GAME_MATCH_DIFFICULTY_NORMAL,
	EM_GAME_MATCH_DIFFICULTY_HARD
} em_game_match_difficulty_t;

/*****************************************************************************/
/* Match display snapshot */
/*****************************************************************************/

typedef struct
{
	uint8_t round;
	uint8_t goals;
	uint8_t saves;
	uint8_t misses;
	uint8_t countdown;
	uint8_t state;
	uint8_t difficulty;
	uint8_t best_goals;
	uint8_t best_difficulty;
	uint8_t last_result;
	uint8_t winner;
} em_game_match_view_t;

static_assert(sizeof(em_game_match_view_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_match_view_t exceeds AK common message payload");

extern void em_game_match_handle(ak_msg_t* msg);

#endif //__EM_GAME_MATCH_H__
