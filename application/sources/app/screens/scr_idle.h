#ifndef __SCR_IDLE_H__
#define __SCR_IDLE_H__

#include <math.h>
#include <stdlib.h>
#include <vector>

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

#include "buzzer.h"

#define SCR_IDLE_MAX_BALL_DISPLAY (16)
#define SCR_IDLE_BALL_MOVE_STEP (2)

extern view_dynamic_t dyn_view_idle;
extern view_screen_t scr_idle;
extern void scr_idle_handle(ak_msg_t* msg);

#endif //__SCR_IDLE_H__
