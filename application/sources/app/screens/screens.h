#ifndef __SCREENS_H__
#define __SCREENS_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include <math.h>
#include <vector>

#include "qrcode.h"
#include "screens_bitmap.h"

// scr_idle
extern view_dynamic_t dyn_view_idle;
extern view_screen_t scr_idle;
extern void scr_idle_handle(ak_msg_t* msg);

// scr_qrcode
extern view_dynamic_t dyn_view_qrcode;
extern view_screen_t scr_qrcode;
extern void scr_qrcode_handle(ak_msg_t* msg);

// scr_startup
extern view_dynamic_t dyn_view_startup;
extern view_screen_t scr_startup;
extern void scr_startup_handle(ak_msg_t* msg);

// scr_welcome
extern view_dynamic_t dyn_view_welcome;
extern view_screen_t scr_welcome;
extern void scr_welcome_handle(ak_msg_t* msg);

#endif //__SCREENS_H__
