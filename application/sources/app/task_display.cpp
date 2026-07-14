#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_display.h"

#include "screens.h"

scr_mng_t scr_mng_app;

void task_display(ak_msg_t* msg) {
	switch (msg->sig) {
	case EM_GAME_DISPLAY_SHOW_MENU: {
		APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_MENU\n");
		SCREEN_TRAN(scr_game_menu_handle, &scr_game_menu);
	} break;

	case EM_GAME_DISPLAY_SHOW_PENALTY: {
		APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_PENALTY\n");
		SCREEN_TRAN(scr_game_penalty_handle, &scr_game_penalty);
	} break;

	case EM_GAME_DISPLAY_SHOW_GAME_OVER: {
		APP_DBG_SIG("EM_GAME_DISPLAY_SHOW_GAME_OVER\n");
		SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
	} break;

	case EM_GAME_DISPLAY_REFRESH: {
		APP_DBG_SIG("EM_GAME_DISPLAY_REFRESH\n");
		return;
	}

	default:
		break;
	}

	scr_mng_dispatch(msg);
}
