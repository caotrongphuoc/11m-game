#include "ak.h"
#include "message.h"
#include "port.h"

#include "sys_ctrl.h"

#include "app.h"
#include "task_list.h"

#include "em_game_goal.h"
#include "em_game_keeper.h"

static em_game_keeper_view_t s_keeper;
static em_game_keeper_difficulty_t s_difficulty =
    EM_GAME_KEEPER_DIFFICULTY_NORMAL;
static int16_t s_target_x;
static uint32_t s_random_seed;
static uint8_t s_update_elapsed_ms;

static uint32_t em_game_keeper_xorshift32()
{
	uint32_t value = s_random_seed;

	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	s_random_seed = value;

	return value;
}

static void em_game_keeper_publish_view()
{
	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_KEEPER,
	                     (uint8_t*)&s_keeper, sizeof(s_keeper));
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
	s_keeper.x = EM_GAME_KEEPER_START_X;
	s_keeper.y = EM_GAME_KEEPER_START_Y;
	s_keeper.frame = 0;
	s_keeper.visible = true;
	s_keeper.moving = false;
	s_keeper.dive = EM_GAME_KEEPER_DIVE_NONE;
	s_target_x = EM_GAME_KEEPER_START_X;
	s_update_elapsed_ms = 0;

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

	s_random_seed ^= sys_ctrl_millis();
	if (s_random_seed == 0)
	{
		s_random_seed = EM_GAME_KEEPER_RANDOM_FALLBACK;
	}

	dive = (em_game_keeper_dive_t)((em_game_keeper_xorshift32() %
	                                EM_GAME_KEEPER_DIRECTION_COUNT) +
	                               EM_GAME_KEEPER_DIVE_LEFT);

	switch (s_difficulty)
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
	em_game_keeper_dive_t dive = em_game_keeper_pick_dive(shot);

	s_keeper.dive = (uint8_t)dive;
	s_keeper.frame = 0;
	s_keeper.visible = true;
	s_keeper.moving = true;
	s_target_x = em_game_keeper_get_target_x(dive);
	s_update_elapsed_ms = 0;

	em_game_keeper_publish_view();
}

static void em_game_keeper_report_ready()
{
	em_game_goal_keeper_t keeper;

	switch ((em_game_keeper_dive_t)s_keeper.dive)
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
	if (!s_keeper.moving)
	{
		return;
	}

	s_keeper.frame++;
	s_keeper.x = EM_GAME_KEEPER_START_X +
	             ((s_target_x - EM_GAME_KEEPER_START_X) * s_keeper.frame) /
	                 EM_GAME_KEEPER_STEP_COUNT;

	if (s_keeper.frame >= EM_GAME_KEEPER_STEP_COUNT)
	{
		s_keeper.x = s_target_x;
		s_keeper.moving = false;
		em_game_keeper_report_ready();
	}

	em_game_keeper_publish_view();
}

void em_game_keeper_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_KEEPER_SETUP:
	case EM_GAME_KEEPER_RESET:
		em_game_keeper_reset();
		break;

	case EM_GAME_KEEPER_SET_DIFFICULTY:
		if (get_data_len_common_msg(msg) == sizeof(uint8_t))
		{
			uint8_t difficulty = *get_data_common_msg(msg);

			if (difficulty <= EM_GAME_KEEPER_DIFFICULTY_HARD)
			{
				s_difficulty = (em_game_keeper_difficulty_t)difficulty;
			}
		}
		break;

	case EM_GAME_KEEPER_REACT:
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
		if (s_keeper.moving)
		{
			s_update_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (s_update_elapsed_ms >= EM_GAME_KEEPER_ANIM_TICK_INTERVAL)
			{
				s_update_elapsed_ms -= EM_GAME_KEEPER_ANIM_TICK_INTERVAL;
				em_game_keeper_advance();
			}
		}
		break;

	default:
		break;
	}
}
