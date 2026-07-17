#include "ak.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "app.h"
#include "task_list.h"

#include "em_game_shooter.h"

em_game_shooter_t em_game_shooter;

static void em_game_shooter_reset()
{
	timer_remove_attr(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_ANIM_TICK);

	em_game_shooter.x = EM_GAME_SHOOTER_START_X;
	em_game_shooter.y = EM_GAME_SHOOTER_START_Y;
	em_game_shooter.frame = 0;
	em_game_shooter.visible = true;
	em_game_shooter.moving = false;
	em_game_shooter.direction = EM_GAME_DIRECTION_NONE;

}

static void em_game_shooter_start(em_game_direction_t direction)
{
	em_game_shooter.frame = 0;
	em_game_shooter.moving = true;
	em_game_shooter.visible = true;
	em_game_shooter.direction = direction;

	timer_set(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_ANIM_TICK, EM_GAME_SHOOTER_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
}

static void em_game_shooter_advance()
{
	if (!em_game_shooter.moving)
	{
		return;
	}

	em_game_shooter.frame++;

	if (em_game_shooter.frame >= EM_GAME_SHOOTER_STEP_COUNT)
	{
		em_game_shooter.frame = EM_GAME_SHOOTER_STEP_COUNT;
		em_game_shooter.moving = false;
		timer_remove_attr(EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_ANIM_TICK);
	}

}

void em_game_shooter_handle(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case EM_GAME_SHOOTER_RESET:
		em_game_shooter_reset();
		break;

	case EM_GAME_SHOOTER_KICK:
		if (get_data_len_common_msg(msg) == sizeof(uint8_t))
		{
			em_game_direction_t direction =
			    (em_game_direction_t)(*get_data_common_msg(msg));

			if ((direction >= EM_GAME_DIRECTION_LEFT) &&
			    (direction <= EM_GAME_DIRECTION_RIGHT))
			{
				em_game_shooter_start(direction);
			}
		}
		break;

	case EM_GAME_SHOOTER_ANIM_TICK:
		em_game_shooter_advance();
		break;

	default:
		break;
	}
}
