#include "ak.h"
#include "message.h"
#include "port.h"

#include "sys_ctrl.h"

#include "app.h"
#include "task_list.h"

#include "em_game_ball.h"
#include "em_game_keeper.h"
#include "em_game_shooter.h"

static em_game_shooter_view_t s_shooter;
static uint32_t s_random_seed;
static uint32_t s_selection_start_tick;
static uint8_t s_update_elapsed_ms;
static bool s_kick_started;

static uint32_t em_game_shooter_xorshift32()
{
	uint32_t value = s_random_seed;

	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	s_random_seed = value;

	return value;
}

static void em_game_shooter_publish_view()
{
	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_SHOOTER,
	                     (uint8_t*)&s_shooter, sizeof(s_shooter));
}

static void em_game_shooter_reset()
{
	s_shooter.x = EM_GAME_SHOOTER_START_X;
	s_shooter.y = EM_GAME_SHOOTER_START_Y;
	s_shooter.frame = 0;
	s_shooter.visible = true;
	s_shooter.moving = false;
	s_shooter.kick = EM_GAME_SHOOTER_KICK_NONE;
	s_selection_start_tick = sys_ctrl_millis();
	s_update_elapsed_ms = 0;
	s_kick_started = false;

	if (s_random_seed == 0)
	{
		s_random_seed = s_selection_start_tick;
		if (s_random_seed == 0)
		{
			s_random_seed = EM_GAME_SHOOTER_RANDOM_FALLBACK;
		}
	}

	em_game_shooter_publish_view();
}

static em_game_ball_target_t
em_game_shooter_pick_ball_target(em_game_shooter_kick_t kick)
{
	uint32_t reaction_time = sys_ctrl_millis() - s_selection_start_tick;
	uint8_t miss_chance = EM_GAME_SHOOTER_MISS_BASE_CHANCE;

	if (reaction_time < EM_GAME_SHOOTER_FAST_REACTION_INTERVAL)
	{
		miss_chance += EM_GAME_SHOOTER_MISS_FAST_BONUS;
	}

	if (miss_chance > EM_GAME_SHOOTER_MISS_MAX_CHANCE)
	{
		miss_chance = EM_GAME_SHOOTER_MISS_MAX_CHANCE;
	}

	if ((em_game_shooter_xorshift32() % 100) < miss_chance)
	{
		if (kick == EM_GAME_SHOOTER_KICK_LEFT)
		{
			return EM_GAME_BALL_TARGET_WIDE_LEFT;
		}
		if (kick == EM_GAME_SHOOTER_KICK_RIGHT)
		{
			return EM_GAME_BALL_TARGET_WIDE_RIGHT;
		}

		return ((em_game_shooter_xorshift32() & 1U) == 0U)
		           ? EM_GAME_BALL_TARGET_WIDE_LEFT
		           : EM_GAME_BALL_TARGET_WIDE_RIGHT;
	}

	switch (kick)
	{
	case EM_GAME_SHOOTER_KICK_LEFT:
		return EM_GAME_BALL_TARGET_LEFT;

	case EM_GAME_SHOOTER_KICK_RIGHT:
		return EM_GAME_BALL_TARGET_RIGHT;

	case EM_GAME_SHOOTER_KICK_CENTER:
	default:
		return EM_GAME_BALL_TARGET_CENTER;
	}
}

static void em_game_shooter_start(em_game_shooter_kick_t kick)
{
	em_game_ball_kick_t ball_kick;
	em_game_keeper_react_t keeper_react;

	if (s_kick_started)
	{
		return;
	}

	s_shooter.frame = 0;
	s_shooter.moving = true;
	s_shooter.visible = true;
	s_shooter.kick = (uint8_t)kick;
	s_update_elapsed_ms = 0;
	s_kick_started = true;

	ball_kick.target = (uint8_t)em_game_shooter_pick_ball_target(kick);

	switch (kick)
	{
	case EM_GAME_SHOOTER_KICK_LEFT:
		keeper_react.shot_zone = EM_GAME_KEEPER_SHOT_LEFT;
		break;

	case EM_GAME_SHOOTER_KICK_RIGHT:
		keeper_react.shot_zone = EM_GAME_KEEPER_SHOT_RIGHT;
		break;

	case EM_GAME_SHOOTER_KICK_CENTER:
	default:
		keeper_react.shot_zone = EM_GAME_KEEPER_SHOT_CENTER;
		break;
	}

	task_post_common_msg(EM_GAME_BALL_ID, EM_GAME_BALL_KICK,
	                     (uint8_t*)&ball_kick, sizeof(ball_kick));
	task_post_common_msg(EM_GAME_KEEPER_ID, EM_GAME_KEEPER_REACT,
	                     (uint8_t*)&keeper_react, sizeof(keeper_react));
	task_post_pure_msg(AC_TASK_BUZZER_ID, EM_GAME_BUZZER_KICK);
	em_game_shooter_publish_view();
}

static void em_game_shooter_advance()
{
	if (!s_shooter.moving)
	{
		return;
	}

	s_shooter.frame++;

	if (s_shooter.frame >= EM_GAME_SHOOTER_STEP_COUNT)
	{
		s_shooter.frame = EM_GAME_SHOOTER_STEP_COUNT;
		s_shooter.moving = false;
	}

	em_game_shooter_publish_view();
}

void em_game_shooter_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_SHOOTER_SETUP:
	case EM_GAME_SHOOTER_RESET:
		em_game_shooter_reset();
		break;

	case EM_GAME_SHOOTER_REQUEST_KICK_LEFT:
		em_game_shooter_start(EM_GAME_SHOOTER_KICK_LEFT);
		break;

	case EM_GAME_SHOOTER_REQUEST_KICK_CENTER:
		em_game_shooter_start(EM_GAME_SHOOTER_KICK_CENTER);
		break;

	case EM_GAME_SHOOTER_REQUEST_KICK_RIGHT:
		em_game_shooter_start(EM_GAME_SHOOTER_KICK_RIGHT);
		break;

	case EM_GAME_SHOOTER_UPDATE:
		if (s_shooter.moving)
		{
			s_update_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (s_update_elapsed_ms >= EM_GAME_SHOOTER_ANIM_TICK_INTERVAL)
			{
				s_update_elapsed_ms -= EM_GAME_SHOOTER_ANIM_TICK_INTERVAL;
				em_game_shooter_advance();
			}
		}
		break;

	default:
		break;
	}
}
