#ifndef __TASK_DISPLAY_H__
#define __TASK_DISPLAY_H__

#include "screen_manager.h"
#include "screens.h"

#include "game/game_eleven_meter/em_game_ball.h"
#include "game/game_eleven_meter/em_game_keeper.h"
#include "game/game_eleven_meter/em_game_match.h"
#include "game/game_eleven_meter/em_game_scoreboard.h"
#include "game/game_eleven_meter/em_game_shooter.h"

#define EM_GAME_VIEW_FLAG_BALL_VISIBLE (1 << 0)
#define EM_GAME_VIEW_FLAG_KEEPER_VISIBLE (1 << 1)
#define EM_GAME_VIEW_FLAG_SHOOTER_VISIBLE (1 << 2)

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
	uint8_t shooter_frame;
	uint8_t shooter_kick;
	uint8_t keeper_dive;
	uint8_t last_result;
	uint8_t winner;
	int16_t ball_x;
	int16_t ball_y;
	int16_t keeper_x;
	int16_t keeper_y;
	int16_t shooter_x;
	int16_t shooter_y;
	uint8_t flags;
} em_game_view_t;

static_assert(sizeof(em_game_view_t) <= AK_COMMON_MSG_DATA_SIZE,
              "em_game_view_t exceeds AK common message payload");

#define LCD_WIDTH (128)
#define LCD_HEIGHT (64)

#define NUMFLAKES (10)
#define XPOS (0)
#define YPOS (1)
#define DELTAY (2)
#define LOGO16_GLCD_HEIGHT (16)
#define LOGO16_GLCD_WIDTH (16)

extern scr_mng_t scr_mng_app;

extern const em_game_view_t* task_display_get_game_view();

#endif // __TASK_DISPLAY_H__
