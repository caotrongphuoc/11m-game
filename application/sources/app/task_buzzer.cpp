#include "ak.h"
#include "app.h"
#include "app_dbg.h"
#include "buzzer.h"
#include "task_buzzer.h"

void task_buzzer(ak_msg_t* msg)
{
	switch (msg->sig)
	{
	case AC_BUZZER_ALARM:
	{
		APP_DBG_SIG("AC_BUZZER_ALARM\n");
		BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
	}
	break;

	case EM_GAME_BUZZER_KICK:
	{
		APP_DBG_SIG("EM_GAME_BUZZER_KICK\n");
		BUZZER_PlaySound(BUZZER_SOUND_EM_GAME_KICK);
	}
	break;

	case EM_GAME_BUZZER_GOAL:
	{
		APP_DBG_SIG("EM_GAME_BUZZER_GOAL\n");
		BUZZER_PlaySound(BUZZER_SOUND_EM_GAME_GOAL);
	}
	break;

	case EM_GAME_BUZZER_SAVE:
	{
		APP_DBG_SIG("EM_GAME_BUZZER_SAVE\n");
		BUZZER_PlaySound(BUZZER_SOUND_EM_GAME_SAVE);
	}
	break;

	case EM_GAME_BUZZER_MISS:
	{
		APP_DBG_SIG("EM_GAME_BUZZER_MISS\n");
		BUZZER_PlaySound(BUZZER_SOUND_EM_GAME_MISS);
	}
	break;

	case EM_GAME_BUZZER_WIN:
	{
		APP_DBG_SIG("EM_GAME_BUZZER_WIN\n");
		BUZZER_PlaySound(BUZZER_SOUND_EM_GAME_WIN);
	}
	break;

	case EM_GAME_BUZZER_LOSE:
	{
		APP_DBG_SIG("EM_GAME_BUZZER_LOSE\n");
		BUZZER_PlaySound(BUZZER_SOUND_EM_GAME_LOSE);
	}
	break;

	default:
		break;
	}
}
