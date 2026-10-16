#include "em_game_match.h"

/*****************************************************************************/
/* Variable declaration - Match */
/*****************************************************************************/

em_game_scoreboard_t em_game_scoreboard = {};
em_game_match_t em_game_match = {
    EM_GAME_MATCH_DIFFICULTY_NORMAL,
    EM_GAME_GOAL_RESULT_NONE,
    EM_GAME_MATCH_STATE_MENU,
    0,
    EM_GAME_MATCH_DIFFICULTY_EASY,
    0, 0, false};

/*****************************************************************************/
/* Private helpers - Match */
/*****************************************************************************/

static void em_game_match_set_state(em_game_match_state_t state)
{
	em_game_match.state = state;
	em_game_match.state_elapsed_ms = 0;
}

static void em_game_match_publish_view()
{
	em_game_match_view_t view;

	view.round = em_game_scoreboard.round;
	view.goals = em_game_scoreboard.goals;
	view.saves = em_game_scoreboard.saves;
	view.misses = em_game_scoreboard.misses;
	view.countdown = em_game_match.countdown_seconds;
	view.state = (uint8_t)em_game_match.state;
	view.difficulty = (uint8_t)em_game_match.difficulty;
	view.best_goals = em_game_match.best_goals;
	view.best_difficulty = (uint8_t)em_game_match.best_difficulty;
	view.last_result = (uint8_t)em_game_match.last_result;
	view.winner = (uint8_t)em_game_scoreboard.winner;

	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_MATCH,
	                     (uint8_t*)&view, sizeof(view));
}

static void em_game_match_set_keeper_difficulty()
{
	em_game_keeper_difficulty_msg_t difficulty_msg;

	switch (em_game_match.difficulty)
	{
	case EM_GAME_MATCH_DIFFICULTY_EASY:
		difficulty_msg.difficulty = EM_GAME_KEEPER_DIFFICULTY_EASY;
		break;

	case EM_GAME_MATCH_DIFFICULTY_HARD:
		difficulty_msg.difficulty = EM_GAME_KEEPER_DIFFICULTY_HARD;
		break;

	case EM_GAME_MATCH_DIFFICULTY_NORMAL:
	default:
		difficulty_msg.difficulty = EM_GAME_KEEPER_DIFFICULTY_NORMAL;
		break;
	}

	task_post_common_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_SET_DIFFICULTY,
	                     (uint8_t*)&difficulty_msg, sizeof(difficulty_msg));
}

static void em_game_match_cycle_difficulty_left()
{
	if (em_game_match.difficulty == EM_GAME_MATCH_DIFFICULTY_EASY)
	{
		em_game_match.difficulty = EM_GAME_MATCH_DIFFICULTY_HARD;
	}
	else
	{
		em_game_match.difficulty = (em_game_match_difficulty_t)(em_game_match.difficulty - 1);
	}

	em_game_match_set_keeper_difficulty();
	em_game_match_publish_view();
}

static void em_game_match_cycle_difficulty_right()
{
	if (em_game_match.difficulty == EM_GAME_MATCH_DIFFICULTY_HARD)
	{
		em_game_match.difficulty = EM_GAME_MATCH_DIFFICULTY_EASY;
	}
	else
	{
		em_game_match.difficulty = (em_game_match_difficulty_t)(em_game_match.difficulty + 1);
	}

	em_game_match_set_keeper_difficulty();
	em_game_match_publish_view();
}

static void em_game_match_reset()
{
	em_game_scoreboard_reset(&em_game_scoreboard);
	em_game_match.last_result = EM_GAME_GOAL_RESULT_NONE;
	em_game_match.countdown_seconds = 0;
}

static void em_game_match_handle_hit_result(ak_msg_t* msg)
{
	em_game_goal_result_msg_t* result_msg;
	em_game_goal_result_t result;
	uint8_t buzzer_signal;

	if (get_data_len_common_msg(msg) != sizeof(em_game_goal_result_msg_t))
	{
		return;
	}

	result_msg = (em_game_goal_result_msg_t*)get_data_common_msg(msg);
	result = (em_game_goal_result_t)result_msg->result;

	switch (result)
	{
	case EM_GAME_GOAL_RESULT_GOAL:
		buzzer_signal = EM_GAME_BUZZER_GOAL;
		break;

	case EM_GAME_GOAL_RESULT_SAVE:
		buzzer_signal = EM_GAME_BUZZER_SAVE;
		break;

	case EM_GAME_GOAL_RESULT_MISS:
		buzzer_signal = EM_GAME_BUZZER_MISS;
		break;

	default:
		return;
	}

	if (!em_game_scoreboard_record_result(&em_game_scoreboard, result))
	{
		return;
	}

	em_game_match.last_result = result;
	task_post_pure_msg(AC_TASK_BUZZER_ID, buzzer_signal);
	em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_END);
	em_game_match_publish_view();
}

static void em_game_match_finish()
{
	em_game_score_record_t score_record;

	em_game_scoreboard_evaluate_winner(&em_game_scoreboard);

	if ((em_game_scoreboard.goals > em_game_match.best_goals) ||
	    ((em_game_scoreboard.goals == em_game_match.best_goals) &&
	     (em_game_match.difficulty > em_game_match.best_difficulty)))
	{
		em_game_match.best_goals = em_game_scoreboard.goals;
		em_game_match.best_difficulty = em_game_match.difficulty;

		init_score_record(&score_record);
		score_record.best_goals = em_game_match.best_goals;
		score_record.best_difficulty = (uint8_t)em_game_match.best_difficulty;
		save_score_record(&score_record);
	}

	timer_remove_attr(AC_TASK_DISPLAY_ID, EM_GAME_TIME_TICK);
	em_game_match_set_state(EM_GAME_MATCH_STATE_FULL_TIME);
	em_game_match_publish_view();
	task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_FULL_TIME);
	timer_set(EM_GAME_MATCH_ID, EM_GAME_MATCH_FULL_TIME_TIMEOUT,
	          EM_GAME_MATCH_FULL_TIME_DURATION, TIMER_ONE_SHOT);
}

static void em_game_match_show_game_over()
{
	timer_remove_attr(EM_GAME_MATCH_ID, EM_GAME_MATCH_FULL_TIME_TIMEOUT);
	em_game_match_set_state(EM_GAME_MATCH_STATE_GAME_OVER);
	em_game_match_publish_view();
	task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_GAME_OVER);

	if (em_game_scoreboard.winner == EM_GAME_SCOREBOARD_WINNER_PLAYER)
	{
		task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_WIN);
	}
	else
	{
		task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_LOSE);
	}
}

static void em_game_match_request_kick(uint8_t shooter_signal)
{
	em_game_match.countdown_seconds = 0;
	em_game_match_set_state(EM_GAME_MATCH_STATE_REVEAL);
	task_post_pure_msg(EM_GAME_SHOOTER_ID, shooter_signal);
	em_game_match_publish_view();
}

static void em_game_match_initialize()
{
	em_game_score_record_t score_record;

	em_game_match_reset();
	if (load_score_record(&score_record) &&
	    (score_record.best_goals <= EM_GAME_SCOREBOARD_TOTAL_KICKS) &&
	    (score_record.best_difficulty <= EM_GAME_MATCH_DIFFICULTY_HARD))
	{
		em_game_match.best_goals = score_record.best_goals;
		em_game_match.best_difficulty =
		    (em_game_match_difficulty_t)score_record.best_difficulty;
	}
	else
	{
		em_game_match.best_goals = 0;
		em_game_match.best_difficulty = EM_GAME_MATCH_DIFFICULTY_EASY;
	}

	em_game_match_set_state(EM_GAME_MATCH_STATE_MENU);
	em_game_match_set_keeper_difficulty();
	em_game_match_publish_view();
	em_game_match.initialized = true;
}

/*****************************************************************************/
/* Handle - Match */
/*****************************************************************************/

void em_game_match_handle(ak_msg_t* msg)
{
	if (!em_game_match.initialized)
	{
		em_game_match_initialize();
	}

	switch (msg->sig)
	{
	case EM_GAME_MATCH_INIT:
		APP_DBG_SIG("EM_GAME_MATCH_INIT\n");
		break;

	case EM_GAME_MATCH_SETUP:
		APP_DBG_SIG("EM_GAME_MATCH_SETUP\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_ROUND_START)
		{
			if (!em_game_scoreboard_advance_round(&em_game_scoreboard))
			{
				break;
			}

			em_game_match.last_result = EM_GAME_GOAL_RESULT_NONE;

			if (em_game_scoreboard.round > 1)
			{
				task_post_pure_msg(EM_GAME_GOAL_ID, EM_GAME_GOAL_RESET);
				task_post_pure_msg(EM_GAME_BALL_ID, EM_GAME_BALL_RESET);
				task_post_pure_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_RESET);
				task_post_pure_msg(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_RESET);
			}

			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_START_ROUND);
			em_game_match_publish_view();
		}
		break;

	case EM_GAME_MATCH_START_ROUND:
		APP_DBG_SIG("EM_GAME_MATCH_START_ROUND\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_ROUND_START)
		{
			em_game_match.countdown_seconds = 3;
			em_game_match_set_state(EM_GAME_MATCH_STATE_SHOOTER_WAIT);
			if (em_game_scoreboard.round == 1)
			{
				task_post_pure_msg(AC_TASK_DISPLAY_ID,
				                   EM_GAME_DISPLAY_SHOW_PENALTY);
			}
			em_game_match_publish_view();
		}
		break;

	case EM_GAME_MATCH_START:
		APP_DBG_SIG("EM_GAME_MATCH_START\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_MENU)
		{
			em_game_match_reset();
			em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_START);
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		}
		break;

	case EM_GAME_MATCH_MENU_LEFT:
		APP_DBG_SIG("EM_GAME_MATCH_MENU_LEFT\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_MENU)
		{
			em_game_match_cycle_difficulty_left();
		}
		break;

	case EM_GAME_MATCH_MENU_RIGHT:
		APP_DBG_SIG("EM_GAME_MATCH_MENU_RIGHT\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_MENU)
		{
			em_game_match_cycle_difficulty_right();
		}
		break;

	case EM_GAME_MATCH_KICK_LEFT:
		APP_DBG_SIG("EM_GAME_MATCH_KICK_LEFT\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			em_game_match_request_kick(EM_GAME_SHOOTER_REQUEST_KICK_LEFT);
		}
		break;

	case EM_GAME_MATCH_KICK_CENTER:
		APP_DBG_SIG("EM_GAME_MATCH_KICK_CENTER\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			em_game_match_request_kick(EM_GAME_SHOOTER_REQUEST_KICK_CENTER);
		}
		break;

	case EM_GAME_MATCH_KICK_RIGHT:
		APP_DBG_SIG("EM_GAME_MATCH_KICK_RIGHT\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			em_game_match_request_kick(EM_GAME_SHOOTER_REQUEST_KICK_RIGHT);
		}
		break;

	case EM_GAME_MATCH_RETRY:
		APP_DBG_SIG("EM_GAME_MATCH_RETRY\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_GAME_OVER)
		{
			em_game_match_reset();
			em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_START);
			task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_SETUP);
		}
		break;

	case EM_GAME_MATCH_HOME:
		APP_DBG_SIG("EM_GAME_MATCH_HOME\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_GAME_OVER)
		{
			em_game_match_reset();
			em_game_match_set_state(EM_GAME_MATCH_STATE_MENU);
			em_game_match_publish_view();
			task_post_pure_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_SHOW_MENU);
		}
		break;

	case EM_GAME_MATCH_UPDATE:
		if (em_game_match.state == EM_GAME_MATCH_STATE_SHOOTER_WAIT)
		{
			uint8_t countdown_seconds;

			em_game_match.state_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (em_game_match.state_elapsed_ms >= EM_GAME_MATCH_SELECTION_TIMEOUT)
			{
				em_game_match_request_kick(
				    EM_GAME_SHOOTER_REQUEST_KICK_CENTER);
				break;
			}

			countdown_seconds =
			    (uint8_t)((EM_GAME_MATCH_SELECTION_TIMEOUT -
			               em_game_match.state_elapsed_ms + 999U) /
			              1000U);
			if (countdown_seconds != em_game_match.countdown_seconds)
			{
				em_game_match.countdown_seconds = countdown_seconds;
				em_game_match_publish_view();
			}
		}
		else if (em_game_match.state == EM_GAME_MATCH_STATE_ROUND_END)
		{
			em_game_match.state_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (em_game_match.state_elapsed_ms < EM_GAME_MATCH_ROUND_END_INTERVAL)
			{
				break;
			}

			if (em_game_scoreboard_is_complete(&em_game_scoreboard))
			{
				em_game_match_finish();
			}
			else
			{
				em_game_match_set_state(EM_GAME_MATCH_STATE_ROUND_START);
				task_post_pure_msg(EM_GAME_MATCH_ID,
				                   EM_GAME_MATCH_SETUP);
			}
		}
		break;

	case EM_GAME_MATCH_HIT_RESULT:
		APP_DBG_SIG("EM_GAME_MATCH_HIT_RESULT\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_REVEAL)
		{
			em_game_match_handle_hit_result(msg);
		}
		break;

	case EM_GAME_MATCH_FULL_TIME_TIMEOUT:
	case EM_GAME_MATCH_SKIP_FULL_TIME:
		APP_DBG_SIG("EM_GAME_MATCH_FULL_TIME_COMPLETE\n");
		if (em_game_match.state == EM_GAME_MATCH_STATE_FULL_TIME)
		{
			em_game_match_show_game_over();
		}
		break;

	default:
		break;
	}
}
