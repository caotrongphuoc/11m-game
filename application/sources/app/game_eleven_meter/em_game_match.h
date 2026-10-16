#ifndef __EM_GAME_MATCH_H__
#define __EM_GAME_MATCH_H__

#include <stdint.h>

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"

#include "em_game_goal.h"
#include "em_game_keeper.h"
#include "em_game_scoreboard.h"

// Match state and difficulty

typedef enum
{
	EM_GAME_MATCH_STATE_MENU,
	EM_GAME_MATCH_STATE_ROUND_START,
	EM_GAME_MATCH_STATE_SHOOTER_WAIT,
	EM_GAME_MATCH_STATE_REVEAL,
	EM_GAME_MATCH_STATE_ROUND_END,
	EM_GAME_MATCH_STATE_GAME_OVER,
	EM_GAME_MATCH_STATE_FULL_TIME
} em_game_match_state_t;

typedef enum
{
	EM_GAME_MATCH_DIFFICULTY_EASY,
	EM_GAME_MATCH_DIFFICULTY_NORMAL,
	EM_GAME_MATCH_DIFFICULTY_HARD
} em_game_match_difficulty_t;

// Match display snapshot

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

// Object state, written only by its owning task.
typedef struct
{
	em_game_match_difficulty_t difficulty;
	em_game_goal_result_t last_result;
	em_game_match_state_t state;
	uint8_t best_goals;
	em_game_match_difficulty_t best_difficulty;
	uint8_t countdown_seconds;
	uint16_t state_elapsed_ms;
	bool initialized;
} em_game_match_t;

extern em_game_match_t em_game_match;

extern void em_game_match_handle(ak_msg_t* msg);

#endif //__EM_GAME_MATCH_H__
