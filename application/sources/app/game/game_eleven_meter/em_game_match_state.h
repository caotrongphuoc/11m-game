#ifndef __EM_GAME_MATCH_STATE_H__
#define __EM_GAME_MATCH_STATE_H__

#include <stdbool.h>
#include <stdint.h>

#include "ak.h"
#include "port.h"
#include "message.h"

#include "em_game_types.h"
#include "em_game_scoreboard.h"

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t target_x;
	int16_t target_y;
	uint8_t frame;
	bool visible;
	bool moving;
	bool wide;
	em_game_direction_t direction;
} em_game_ball_t;

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t target_x;
	int16_t target_y;
	uint8_t frame;
	bool visible;
	bool moving;
	em_game_direction_t direction;
} em_game_keeper_t;

typedef struct
{
	int16_t x;
	int16_t y;
	uint8_t frame;
	bool visible;
	em_game_direction_t direction;
} em_game_shooter_t;

typedef struct
{
	em_game_ball_t ball;
	em_game_keeper_t keeper;
	em_game_shooter_t shooter;
	em_game_scoreboard_t scoreboard;
	em_game_direction_t pending_direction;
	em_game_result_t last_result;
	uint32_t random_seed;
	uint32_t countdown_start_tick;
	uint8_t countdown_seconds;
	bool input_locked;
} em_game_match_state_t;

typedef struct
{
	uint8_t screen;
	uint8_t state;
	uint8_t round;
	uint8_t goals;
	uint8_t saves;
	uint8_t misses;
	uint8_t countdown;
	uint8_t shooter_direction;
	uint8_t keeper_direction;
	uint8_t last_result;
	uint8_t winner;
	int16_t ball_x;
	int16_t ball_y;
	uint8_t ball_frame;
	uint8_t keeper_frame;
	uint8_t shooter_frame;
	uint8_t flags;
} em_game_view_t;

static_assert(sizeof(em_game_view_t) <= AK_COMMON_MSG_DATA_SIZE, "em_game_view_t exceeds AK common message payload");

extern void em_game_match_state_init(em_game_match_state_t* ms);
extern void em_game_match_state_reset(em_game_match_state_t* ms);
extern void em_game_match_state_build_view(const em_game_match_state_t* ms, em_game_view_t* view);

#endif //__EM_GAME_MATCH_STATE_H__
