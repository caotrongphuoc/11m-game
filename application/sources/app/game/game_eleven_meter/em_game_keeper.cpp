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

	task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_DISPLAY_REFRESH);
}

static void em_game_keeper_pick_direction()
{
	uint32_t tick = sys_ctrl_millis();

	s_keeper_random_seed ^= tick;
	if (s_keeper_random_seed == 0)
	{
		s_keeper_random_seed = EM_GAME_KEEPER_RANDOM_FALLBACK;
	}

	em_game_keeper.direction =
	    (em_game_direction_t)((em_game_keeper_xorshift32() % 3) + EM_GAME_DIRECTION_LEFT);
	em_game_keeper.target_x = em_game_keeper_get_target_x(em_game_keeper.direction);
	em_game_keeper.target_y = EM_GAME_KEEPER_TARGET_Y;
	em_game_keeper.frame = 0;
	em_game_keeper.moving = true;
	em_game_keeper.visible = true;

	timer_set(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_ANIM_TICK, EM_GAME_KEEPER_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
	task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_DISPLAY_REFRESH);
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

	task_post_pure_msg(EM_GAME_MATCH_ID, EM_GAME_MATCH_DISPLAY_REFRESH);
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
		em_game_keeper_pick_direction();
		break;

	case EM_GAME_KEEPER_ANIM_TICK:
		em_game_keeper_advance();
		break;

	default:
		break;
	}
}
