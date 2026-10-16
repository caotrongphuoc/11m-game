#ifndef __SCR_STARTUP_H__
#define __SCR_STARTUP_H__

#include "fsm.h"
#include "message.h"
#include "port.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"
#include "screen_manager.h"

#include "io_cfg.h"
#include "scr_game_menu.h"
#include "scr_idle.h"

#define SCR_STARTUP_LOGO_AXIS_X (23)
#define SCR_STARTUP_LOGO_TEXT_AXIS_X (SCR_STARTUP_LOGO_AXIS_X + 4)

extern view_dynamic_t dyn_view_startup;
extern view_screen_t scr_startup;
extern void scr_startup_handle(ak_msg_t* msg);

#endif //__SCR_STARTUP_H__
