#ifndef __SCREENS_BITMAP_H__
#define __SCREENS_BITMAP_H__

#include "view_render.h"

/*
 * Penalty sprites are sized for the 128x64 OLED layout:
 * - The 7x7 ball remains visible without crowding a 23-pixel goal zone.
 * - The idle and dive keeper frames are 30x30 pixels.
 * - The shooter frames are 30x30 pixels.
 */
extern const unsigned char PROGMEM bitmap_penalty_ball[];
extern const unsigned char PROGMEM bitmap_penalty_keeper_idle[];
extern const unsigned char PROGMEM bitmap_penalty_keeper_dive[];
extern const unsigned char PROGMEM bitmap_penalty_shooter_prepare[];
extern const unsigned char PROGMEM bitmap_penalty_shooter_kick[];
extern const unsigned char PROGMEM bitmap_penalty_shooter_follow[];
extern const unsigned char PROGMEM bitmap_penalty_goal[];

// scr_welcome
extern const unsigned char PROGMEM bitmap_dolphin[];

#endif //__SCREENS_BITMAP_H__
