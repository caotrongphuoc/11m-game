#include <string.h>

#include "em_game_match_state.h"

void em_game_match_state_init(em_game_match_state_t* ms)
{
	memset(ms, 0, sizeof(*ms));
	em_game_scoreboard_reset(&ms->scoreboard);
}

void em_game_match_state_reset(em_game_match_state_t* ms)
{
	uint32_t random_seed = ms->random_seed;

	em_game_match_state_init(ms);
	ms->random_seed = random_seed;
}

void em_game_match_state_build_view(const em_game_match_state_t* ms, em_game_view_t* view)
{
	memset(view, 0, sizeof(em_game_view_t));

	view->round = ms->scoreboard.round;
	view->goals = ms->scoreboard.goals;
	view->saves = ms->scoreboard.saves;
	view->misses = ms->scoreboard.misses;
	view->countdown = ms->countdown_seconds;
	view->shooter_direction = (uint8_t)ms->shooter.direction;
	view->keeper_direction = (uint8_t)ms->keeper.direction;
	view->last_result = (uint8_t)ms->last_result;
	view->winner = (uint8_t)ms->scoreboard.winner;
	view->ball_x = ms->ball.x;
	view->ball_y = ms->ball.y;
	view->keeper_x = ms->keeper.x;
	view->keeper_y = ms->keeper.y;
	view->shooter_x = ms->shooter.x;
	view->shooter_y = ms->shooter.y;
	view->ball_frame = ms->ball.frame;
	view->keeper_frame = ms->keeper.frame;
	view->shooter_frame = ms->shooter.frame;

	if (ms->ball.visible)
	{
		view->flags |= EM_GAME_VIEW_FLAG_BALL_VISIBLE;
	}
	if (ms->keeper.visible)
	{
		view->flags |= EM_GAME_VIEW_FLAG_KEEPER_VISIBLE;
	}
	if (ms->shooter.visible)
	{
		view->flags |= EM_GAME_VIEW_FLAG_SHOOTER_VISIBLE;
	}
}
