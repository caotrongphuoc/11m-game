#ifndef __SCREENS_BITMAP_H__
#define __SCREENS_BITMAP_H__

#include "view_render.h"

/*
 * Penalty sprites are sized for the 128x64 OLED layout:
 * - The 7x7 ball remains visible without crowding a 23-pixel goal zone.
 * - The 15x20 idle keeper fits inside the 30-pixel goal frame.
 * - The 21x14 dive keeper emphasizes horizontal motion while fitting one zone.
 * - The 15x20 shooter frames remain readable in the lower screen area.
 */
extern const unsigned char PROGMEM bitmap_penalty_ball[];
extern const unsigned char PROGMEM bitmap_penalty_keeper_idle[];
extern const unsigned char PROGMEM bitmap_penalty_keeper_dive[];
extern const unsigned char PROGMEM bitmap_penalty_shooter_prepare[];
extern const unsigned char PROGMEM bitmap_penalty_shooter_kick[];
extern const unsigned char PROGMEM bitmap_penalty_shooter_follow[];

// scr_welcome
extern const unsigned char PROGMEM bitmap_dolphin[];

#endif //__SCREENS_BITMAP_H__
