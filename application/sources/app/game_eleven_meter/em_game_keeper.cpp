#include "em_game_keeper.h"

/*****************************************************************************/
/* Variable declaration - Keeper */
/*****************************************************************************/

em_game_keeper_t em_game_keeper = {
    {}, EM_GAME_KEEPER_DIFFICULTY_NORMAL, 0, 0, 0, false};

/*****************************************************************************/
/* Private helpers - Keeper */
/*****************************************************************************/

static uint32_t em_game_keeper_xorshift32()
{
	uint32_t value = em_game_keeper.random_seed;

	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	em_game_keeper.random_seed = value;

	return value;
}

static void em_game_keeper_publish_view()
{
	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_KEEPER,
	                     (uint8_t*)&em_game_keeper.view, sizeof(em_game_keeper.view));
}

static int16_t em_game_keeper_get_target_x(em_game_keeper_dive_t dive)
{
	switch (dive)
	{
	case EM_GAME_KEEPER_DIVE_LEFT:
		return EM_GAME_KEEPER_TARGET_LEFT_X;

	case EM_GAME_KEEPER_DIVE_RIGHT:
		return EM_GAME_KEEPER_TARGET_RIGHT_X;

	case EM_GAME_KEEPER_DIVE_CENTER:
	default:
		return EM_GAME_KEEPER_TARGET_CENTER_X;
	}
}

static void em_game_keeper_reset()
{
	em_game_keeper.view.x = EM_GAME_KEEPER_START_X;
	em_game_keeper.view.y = EM_GAME_KEEPER_START_Y;
	em_game_keeper.view.frame = 0;
	em_game_keeper.view.visible = true;
	em_game_keeper.view.moving = false;
	em_game_keeper.view.dive = EM_GAME_KEEPER_DIVE_NONE;
	em_game_keeper.target_x = EM_GAME_KEEPER_START_X;
	em_game_keeper.update_elapsed_ms = 0;
	em_game_keeper.reaction_started = false;

	em_game_keeper_publish_view();
}

static em_game_keeper_dive_t
em_game_keeper_shot_to_dive(em_game_keeper_shot_t shot)
{
	switch (shot)
	{
	case EM_GAME_KEEPER_SHOT_LEFT:
		return EM_GAME_KEEPER_DIVE_LEFT;

	case EM_GAME_KEEPER_SHOT_RIGHT:
		return EM_GAME_KEEPER_DIVE_RIGHT;

	case EM_GAME_KEEPER_SHOT_CENTER:
	default:
		return EM_GAME_KEEPER_DIVE_CENTER;
	}
}

static em_game_keeper_dive_t
em_game_keeper_pick_dive(em_game_keeper_shot_t shot)
{
	em_game_keeper_dive_t shot_dive = em_game_keeper_shot_to_dive(shot);
	em_game_keeper_dive_t dive;
	uint8_t roll;

	em_game_keeper.random_seed ^= sys_ctrl_millis();
	if (em_game_keeper.random_seed == 0)
	{
		em_game_keeper.random_seed = EM_GAME_KEEPER_RANDOM_FALLBACK;
	}

	dive = (em_game_keeper_dive_t)((em_game_keeper_xorshift32() %
	                                EM_GAME_KEEPER_DIRECTION_COUNT) +
	                               EM_GAME_KEEPER_DIVE_LEFT);

	switch (em_game_keeper.difficulty)
	{
	case EM_GAME_KEEPER_DIFFICULTY_EASY:
		if (dive == shot_dive)
		{
			roll = em_game_keeper_xorshift32() % 100;
			if (roll < EM_GAME_KEEPER_EASY_MATCH_FLIP_CHANCE)
			{
				uint8_t offset = (em_game_keeper_xorshift32() %
				                  (EM_GAME_KEEPER_DIRECTION_COUNT - 1)) +
				                 1;

				dive = (em_game_keeper_dive_t)(((dive - EM_GAME_KEEPER_DIVE_LEFT +
				                                 offset) %
				                                EM_GAME_KEEPER_DIRECTION_COUNT) +
				                               EM_GAME_KEEPER_DIVE_LEFT);
			}
		}
		break;

	case EM_GAME_KEEPER_DIFFICULTY_HARD:
		if (dive != shot_dive)
		{
			roll = em_game_keeper_xorshift32() % 100;
			if (roll < EM_GAME_KEEPER_HARD_MISS_OVERRIDE_CHANCE)
			{
				dive = shot_dive;
			}
		}
		break;

	case EM_GAME_KEEPER_DIFFICULTY_NORMAL:
	default:
		break;
	}

	return dive;
}

static void em_game_keeper_start(em_game_keeper_shot_t shot)
{
	if (em_game_keeper.reaction_started)
	{
		return;
	}

	em_game_keeper_dive_t dive = em_game_keeper_pick_dive(shot);

	em_game_keeper.view.dive = (uint8_t)dive;
	em_game_keeper.view.frame = 0;
	em_game_keeper.view.visible = true;
	em_game_keeper.view.moving = true;
	em_game_keeper.target_x = em_game_keeper_get_target_x(dive);
	em_game_keeper.update_elapsed_ms = 0;
	em_game_keeper.reaction_started = true;

	em_game_keeper_publish_view();
}

static void em_game_keeper_report_ready()
{
	em_game_goal_keeper_t keeper;

	switch ((em_game_keeper_dive_t)em_game_keeper.view.dive)
	{
	case EM_GAME_KEEPER_DIVE_LEFT:
		keeper.zone = EM_GAME_GOAL_ZONE_LEFT;
		break;

	case EM_GAME_KEEPER_DIVE_RIGHT:
		keeper.zone = EM_GAME_GOAL_ZONE_RIGHT;
		break;

	case EM_GAME_KEEPER_DIVE_CENTER:
	default:
		keeper.zone = EM_GAME_GOAL_ZONE_CENTER;
		break;
	}

	task_post_common_msg(EM_GAME_GOAL_ID, EM_GAME_GOAL_KEEPER_READY,
	                     (uint8_t*)&keeper, sizeof(keeper));
}

static void em_game_keeper_advance()
{
	if (!em_game_keeper.view.moving)
	{
		return;
	}

	em_game_keeper.view.frame++;
	em_game_keeper.view.x = EM_GAME_KEEPER_START_X +
	                        ((em_game_keeper.target_x - EM_GAME_KEEPER_START_X) * em_game_keeper.view.frame) /
	                            EM_GAME_KEEPER_STEP_COUNT;

	if (em_game_keeper.view.frame >= EM_GAME_KEEPER_STEP_COUNT)
	{
		em_game_keeper.view.x = em_game_keeper.target_x;
		em_game_keeper.view.moving = false;
		em_game_keeper_report_ready();
	}

	em_game_keeper_publish_view();
}

/*****************************************************************************/
/* Handle - Keeper */
/*****************************************************************************/

void em_game_keeper_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_KEEPER_SETUP:
		APP_DBG_SIG("EM_GAME_KEEPER_SETUP\n");
		em_game_keeper_reset();
		break;

	case EM_GAME_KEEPER_RESET:
		APP_DBG_SIG("EM_GAME_KEEPER_RESET\n");
		em_game_keeper_reset();
		break;

	case EM_GAME_KEEPER_SET_DIFFICULTY:
		APP_DBG_SIG("EM_GAME_KEEPER_SET_DIFFICULTY\n");
		if (get_data_len_common_msg(msg) ==
		    sizeof(em_game_keeper_difficulty_msg_t))
		{
			em_game_keeper_difficulty_msg_t* difficulty_msg =
			    (em_game_keeper_difficulty_msg_t*)get_data_common_msg(msg);

			if (difficulty_msg->difficulty <= EM_GAME_KEEPER_DIFFICULTY_HARD)
			{
				em_game_keeper.difficulty = (em_game_keeper_difficulty_t)
				                                difficulty_msg->difficulty;
			}
		}
		break;

	case EM_GAME_KEEPER_REACT:
		APP_DBG_SIG("EM_GAME_KEEPER_REACT\n");
		if (get_data_len_common_msg(msg) == sizeof(em_game_keeper_react_t))
		{
			em_game_keeper_react_t* react =
			    (em_game_keeper_react_t*)get_data_common_msg(msg);

			if ((react->shot_zone >= EM_GAME_KEEPER_SHOT_LEFT) &&
			    (react->shot_zone <= EM_GAME_KEEPER_SHOT_RIGHT))
			{
				em_game_keeper_start((em_game_keeper_shot_t)react->shot_zone);
			}
		}
		break;

	case EM_GAME_KEEPER_UPDATE:
		if (em_game_keeper.view.moving)
		{
			em_game_keeper.update_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (em_game_keeper.update_elapsed_ms >= EM_GAME_KEEPER_ANIM_TICK_INTERVAL)
			{
				em_game_keeper.update_elapsed_ms -= EM_GAME_KEEPER_ANIM_TICK_INTERVAL;
				em_game_keeper_advance();
			}
		}
		break;

	default:
		break;
	}
}
