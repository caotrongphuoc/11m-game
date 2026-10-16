#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include <stdint.h>

#include "sys_boot.h"
#include "sys_dbg.h"
#include "app.h"
#include "buzzer.h"

/******************************************************************************
 * IF Type
 *******************************************************************************/
/** APP interface, communication via socket interface
 *
 */
#define IF_TYPE_APP_MIN (100)
#define IF_TYPE_APP_GW (100)
#define IF_TYPE_APP_GI (101)
#define IF_TYPE_APP_MAX (119)

/** UART interface
 *
 */
#define IF_TYPE_UART_GW_MIN (120)
#define IF_TYPE_UART_GW (120)
#define IF_TYPE_UART_AC (121)
#define IF_TYPE_UART_GW_MAX (140)

typedef struct
{
	uint8_t is_power_on_reset;
} boot_app_share_data_t;

#endif //__APP_DATA_H__
