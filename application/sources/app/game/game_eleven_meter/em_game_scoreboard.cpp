#include "em_game_scoreboard.h"

void em_game_scoreboard_reset(em_game_scoreboard_t* sb)
{
	sb->round = 0;
	sb->goals = 0;
	sb->saves = 0;
	sb->misses = 0;
	sb->winner = EM_GAME_WINNER_NONE;
}

void em_game_scoreboard_record_result(em_game_scoreboard_t* sb, em_game_result_t result)
{
	switch (result)
	{
	case EM_GAME_RESULT_GOAL:
		sb->goals++;
		break;

	case EM_GAME_RESULT_SAVE:
		sb->saves++;
		break;

	case EM_GAME_RESULT_MISS:
		sb->misses++;
		break;

	default:
		break;
	}
}

void em_game_scoreboard_advance_round(em_game_scoreboard_t* sb)
{
	if (sb->round < EM_GAME_SCOREBOARD_TOTAL_KICKS)
	{
		sb->round++;
	}
}

bool em_game_scoreboard_is_complete(em_game_scoreboard_t* sb)
{
	return sb->round >= EM_GAME_SCOREBOARD_TOTAL_KICKS;
}

em_game_winner_t em_game_scoreboard_evaluate_winner(em_game_scoreboard_t* sb)
{
	if (!em_game_scoreboard_is_complete(sb))
	{
		sb->winner = EM_GAME_WINNER_NONE;
	}
	else if (sb->goals >= EM_GAME_SCOREBOARD_WIN_THRESHOLD)
	{
		sb->winner = EM_GAME_WINNER_PLAYER;
	}
	else
	{
		sb->winner = EM_GAME_WINNER_AI;
	}

	return sb->winner;
}
