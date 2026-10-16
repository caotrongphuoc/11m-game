#include "em_game_shooter.h"

/*****************************************************************************/
/* Variable declaration - Shooter */
/*****************************************************************************/

em_game_shooter_t em_game_shooter = {};

/*****************************************************************************/
/* Private helpers - Shooter */
/*****************************************************************************/

static uint32_t em_game_shooter_xorshift32()
{
	uint32_t value = em_game_shooter.random_seed;

	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	em_game_shooter.random_seed = value;

	return value;
}

static void em_game_shooter_publish_view()
{
	task_post_common_msg(AC_TASK_DISPLAY_ID, EM_GAME_DISPLAY_UPDATE_SHOOTER,
	                     (uint8_t*)&em_game_shooter.view, sizeof(em_game_shooter.view));
}

static void em_game_shooter_reset()
{
	em_game_shooter.view.x = EM_GAME_SHOOTER_START_X;
	em_game_shooter.view.y = EM_GAME_SHOOTER_START_Y;
	em_game_shooter.view.frame = 0;
	em_game_shooter.view.visible = true;
	em_game_shooter.view.moving = false;
	em_game_shooter.view.kick = EM_GAME_SHOOTER_KICK_NONE;
	em_game_shooter.selection_elapsed_ms = 0;
	em_game_shooter.update_elapsed_ms = 0;
	em_game_shooter.kick_started = false;

	if (em_game_shooter.random_seed == 0)
	{
		em_game_shooter.random_seed = sys_ctrl_millis();
		if (em_game_shooter.random_seed == 0)
		{
			em_game_shooter.random_seed = EM_GAME_SHOOTER_RANDOM_FALLBACK;
		}
	}

	em_game_shooter_publish_view();
}

static em_game_ball_target_t
em_game_shooter_pick_ball_target(em_game_shooter_kick_t kick)
{
	uint8_t miss_chance = EM_GAME_SHOOTER_MISS_BASE_CHANCE;

	if (em_game_shooter.selection_elapsed_ms < EM_GAME_SHOOTER_FAST_REACTION_INTERVAL)
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

	if (em_game_shooter.kick_started)
	{
		return;
	}

	em_game_shooter.view.frame = 0;
	em_game_shooter.view.moving = true;
	em_game_shooter.view.x = EM_GAME_SHOOTER_START_X + EM_GAME_SHOOTER_KICK_STEP_X;
	em_game_shooter.view.visible = true;
	em_game_shooter.view.kick = (uint8_t)kick;
	em_game_shooter.update_elapsed_ms = 0;
	em_game_shooter.kick_started = true;

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
	if (!em_game_shooter.view.moving)
	{
		return;
	}

	em_game_shooter.view.frame++;
	// A small forward step replaces the raised-leg sprite, then settles back.
	em_game_shooter.view.x = EM_GAME_SHOOTER_START_X +
	                         ((EM_GAME_SHOOTER_STEP_COUNT - em_game_shooter.view.frame) *
	                          EM_GAME_SHOOTER_KICK_STEP_X) /
	                             EM_GAME_SHOOTER_STEP_COUNT;

	if (em_game_shooter.view.frame >= EM_GAME_SHOOTER_STEP_COUNT)
	{
		em_game_shooter.view.frame = EM_GAME_SHOOTER_STEP_COUNT;
		em_game_shooter.view.moving = false;
	}

	em_game_shooter_publish_view();
}

/*****************************************************************************/
/* Handle - Shooter */
/*****************************************************************************/

void em_game_shooter_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_SHOOTER_SETUP:
		APP_DBG_SIG("EM_GAME_SHOOTER_SETUP\n");
		em_game_shooter_reset();
		break;

	case EM_GAME_SHOOTER_RESET:
		APP_DBG_SIG("EM_GAME_SHOOTER_RESET\n");
		em_game_shooter_reset();
		break;

	case EM_GAME_SHOOTER_REQUEST_KICK_LEFT:
		APP_DBG_SIG("EM_GAME_SHOOTER_REQUEST_KICK_LEFT\n");
		em_game_shooter_start(EM_GAME_SHOOTER_KICK_LEFT);
		break;

	case EM_GAME_SHOOTER_REQUEST_KICK_CENTER:
		APP_DBG_SIG("EM_GAME_SHOOTER_REQUEST_KICK_CENTER\n");
		em_game_shooter_start(EM_GAME_SHOOTER_KICK_CENTER);
		break;

	case EM_GAME_SHOOTER_REQUEST_KICK_RIGHT:
		APP_DBG_SIG("EM_GAME_SHOOTER_REQUEST_KICK_RIGHT\n");
		em_game_shooter_start(EM_GAME_SHOOTER_KICK_RIGHT);
		break;

	case EM_GAME_SHOOTER_UPDATE:
		if (!em_game_shooter.kick_started &&
		    (em_game_shooter.selection_elapsed_ms <
		     EM_GAME_SHOOTER_FAST_REACTION_INTERVAL))
		{
			em_game_shooter.selection_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
		}

		if (em_game_shooter.view.moving)
		{
			em_game_shooter.update_elapsed_ms += EM_GAME_TIME_TICK_INTERVAL;
			if (em_game_shooter.update_elapsed_ms >= EM_GAME_SHOOTER_ANIM_TICK_INTERVAL)
			{
				em_game_shooter.update_elapsed_ms -= EM_GAME_SHOOTER_ANIM_TICK_INTERVAL;
				em_game_shooter_advance();
			}
		}
		break;

	default:
		break;
	}
}
