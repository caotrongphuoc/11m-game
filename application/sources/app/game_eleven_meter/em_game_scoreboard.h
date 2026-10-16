#ifndef __EM_GAME_SCOREBOARD_H__
#define __EM_GAME_SCOREBOARD_H__

#include <stdbool.h>
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

// Scoreboard rules and state

#define EM_GAME_SCOREBOARD_TOTAL_KICKS (5)
#define EM_GAME_SCOREBOARD_WIN_THRESHOLD (3)

typedef enum
{
	EM_GAME_SCOREBOARD_WINNER_NONE,
	EM_GAME_SCOREBOARD_WINNER_PLAYER,
	EM_GAME_SCOREBOARD_WINNER_AI
} em_game_scoreboard_winner_t;

typedef struct
{
	uint8_t round;
	uint8_t goals;
	uint8_t saves;
	uint8_t misses;
	em_game_scoreboard_winner_t winner;
} em_game_scoreboard_t;

// Scoreboard instance, updated by Match through the helpers below.
extern em_game_scoreboard_t em_game_scoreboard;

extern void em_game_scoreboard_reset(em_game_scoreboard_t* sb);
extern bool em_game_scoreboard_record_result(em_game_scoreboard_t* sb,
                                             em_game_goal_result_t result);
extern bool em_game_scoreboard_advance_round(em_game_scoreboard_t* sb);
extern bool em_game_scoreboard_is_complete(em_game_scoreboard_t* sb);
extern em_game_scoreboard_winner_t
em_game_scoreboard_evaluate_winner(em_game_scoreboard_t* sb);

#endif //__EM_GAME_SCOREBOARD_H__
