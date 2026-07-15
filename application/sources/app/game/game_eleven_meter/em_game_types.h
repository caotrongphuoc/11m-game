#ifndef __EM_GAME_TYPES_H__
#define __EM_GAME_TYPES_H__

/*
 * A direction represents input or movement intent.
 */
typedef enum
{
	EM_GAME_DIRECTION_NONE,
	EM_GAME_DIRECTION_LEFT,
	EM_GAME_DIRECTION_CENTER,
	EM_GAME_DIRECTION_RIGHT
} em_game_direction_t;

typedef enum
{
	EM_GAME_DIFFICULTY_EASY,
	EM_GAME_DIFFICULTY_NORMAL,
	EM_GAME_DIFFICULTY_HARD
} em_game_difficulty_t;

typedef enum
{
	EM_GAME_RESULT_NONE,
	EM_GAME_RESULT_GOAL,
	EM_GAME_RESULT_SAVE,
	EM_GAME_RESULT_MISS
} em_game_result_t;

typedef enum
{
	EM_GAME_WINNER_NONE,
	EM_GAME_WINNER_PLAYER,
	EM_GAME_WINNER_AI
} em_game_winner_t;

#endif //__EM_GAME_TYPES_H__
