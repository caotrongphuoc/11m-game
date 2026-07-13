#include "ak.h"
#include "app.h"
#include "app_dbg.h"
#include "buzzer.h"
#include "task_buzzer.h"

void task_buzzer(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_BUZZER_ALARM: {
		APP_DBG_SIG("AC_BUZZER_ALARM\n");
		BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
	} break;

	default:
		break;
	}
}
