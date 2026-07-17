#include "ak.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"

#include "app.h"
#include "task_list.h"

#include "em_game_keeper.h"

em_game_keeper_t em_game_keeper;

static uint32_t s_keeper_random_seed;

static uint32_t em_game_keeper_xorshift32()
{
	uint32_t value = s_keeper_random_seed;

	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	s_keeper_random_seed = value;

	return value;
}

static int16_t em_game_keeper_get_target_x(em_game_direction_t direction)
{
	switch (direction)
	{
	case EM_GAME_DIRECTION_LEFT:
		return EM_GAME_KEEPER_TARGET_LEFT_X;

	case EM_GAME_DIRECTION_CENTER:
		return EM_GAME_KEEPER_TARGET_CENTER_X;

	case EM_GAME_DIRECTION_RIGHT:
		return EM_GAME_KEEPER_TARGET_RIGHT_X;

	default:
		return EM_GAME_KEEPER_START_X;
	}
}

static void em_game_keeper_reset()
{
	timer_remove_attr(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_ANIM_TICK);

	em_game_keeper.x = EM_GAME_KEEPER_START_X;
	em_game_keeper.y = EM_GAME_KEEPER_START_Y;
	em_game_keeper.target_x = EM_GAME_KEEPER_START_X;
	em_game_keeper.target_y = EM_GAME_KEEPER_START_Y;
	em_game_keeper.frame = 0;
	em_game_keeper.visible = true;
	em_game_keeper.moving = false;
	em_game_keeper.direction = EM_GAME_DIRECTION_NONE;

}

static void em_game_keeper_pick_direction(em_game_direction_t shooter_direction,
                                          em_game_difficulty_t difficulty)
{
	uint32_t tick = sys_ctrl_millis();
	em_game_direction_t direction;
	uint8_t roll;

	s_keeper_random_seed ^= tick;
	if (s_keeper_random_seed == 0)
	{
		s_keeper_random_seed = EM_GAME_KEEPER_RANDOM_FALLBACK;
	}

	direction =
	    (em_game_direction_t)((em_game_keeper_xorshift32() %
	                           EM_GAME_KEEPER_DIRECTION_COUNT) +
	                          EM_GAME_DIRECTION_LEFT);

	switch (difficulty)
	{
	case EM_GAME_DIFFICULTY_EASY:
		if (direction == shooter_direction)
		{
			roll = em_game_keeper_xorshift32() % 100;
			if (roll < EM_GAME_KEEPER_EASY_MATCH_FLIP_CHANCE)
			{
				uint8_t offset =
				    (em_game_keeper_xorshift32() %
				     (EM_GAME_KEEPER_DIRECTION_COUNT - 1)) +
				    1;

				direction =
				    (em_game_direction_t)(((direction -
				                            EM_GAME_DIRECTION_LEFT +
				                            offset) %
				                           EM_GAME_KEEPER_DIRECTION_COUNT) +
				                          EM_GAME_DIRECTION_LEFT);
			}
		}
		break;

	case EM_GAME_DIFFICULTY_HARD:
		if (direction != shooter_direction)
		{
			roll = em_game_keeper_xorshift32() % 100;
			if (roll < EM_GAME_KEEPER_HARD_MISS_OVERRIDE_CHANCE)
			{
				direction = shooter_direction;
			}
		}
		break;

	case EM_GAME_DIFFICULTY_NORMAL:
	default:
		break;
	}

	em_game_keeper.direction = direction;
	em_game_keeper.target_x = em_game_keeper_get_target_x(em_game_keeper.direction);
	em_game_keeper.target_y = EM_GAME_KEEPER_TARGET_Y;
	em_game_keeper.frame = 0;
	em_game_keeper.moving = true;
	em_game_keeper.visible = true;

	timer_set(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_ANIM_TICK, EM_GAME_KEEPER_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
}

static void em_game_keeper_advance()
{
	if (!em_game_keeper.moving)
	{
		return;
	}

	em_game_keeper.frame++;
	em_game_keeper.x = EM_GAME_KEEPER_START_X +
	                   ((em_game_keeper.target_x - EM_GAME_KEEPER_START_X) * em_game_keeper.frame) /
	                       EM_GAME_KEEPER_STEP_COUNT;
	em_game_keeper.y = EM_GAME_KEEPER_START_Y +
	                   ((em_game_keeper.target_y - EM_GAME_KEEPER_START_Y) * em_game_keeper.frame) /
	                       EM_GAME_KEEPER_STEP_COUNT;

	if (em_game_keeper.frame >= EM_GAME_KEEPER_STEP_COUNT)
	{
		em_game_keeper.x = em_game_keeper.target_x;
		em_game_keeper.y = em_game_keeper.target_y;
		em_game_keeper.moving = false;
		timer_remove_attr(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_ANIM_TICK);
	}

}

em_game_direction_t em_game_keeper_get_direction()
{
	return em_game_keeper.direction;
}

void em_game_keeper_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_KEEPER_RESET:
		em_game_keeper_reset();
		break;

	case EM_GAME_KEEPER_AI_PICK:
		if (get_data_len_common_msg(msg) == sizeof(em_game_keeper_ai_pick_t))
		{
			em_game_keeper_ai_pick_t* keeper_pick =
			    (em_game_keeper_ai_pick_t*)get_data_common_msg(msg);
			em_game_direction_t shooter_direction =
			    (em_game_direction_t)keeper_pick->shooter_direction;
			em_game_difficulty_t difficulty =
			    (em_game_difficulty_t)keeper_pick->difficulty;

			if ((shooter_direction >= EM_GAME_DIRECTION_LEFT) &&
			    (shooter_direction <= EM_GAME_DIRECTION_RIGHT) &&
			    (difficulty >= EM_GAME_DIFFICULTY_EASY) &&
			    (difficulty <= EM_GAME_DIFFICULTY_HARD))
			{
				em_game_keeper_pick_direction(shooter_direction,
				                              difficulty);
			}
		}
		break;

	case EM_GAME_KEEPER_ANIM_TICK:
		em_game_keeper_advance();
		break;

	default:
		break;
	}
}
