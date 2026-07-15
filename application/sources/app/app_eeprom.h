#ifndef __APP_EEPROM_H__
#define __APP_EEPROM_H__

#include <stdbool.h>
#include <stdint.h>

/**
 *****************************************************************************
 * EEPROM define address.
 *
 *****************************************************************************
 */
#define EEPROM_START_ADDR (0X0000)
#define EEPROM_END_ADDR (0X1000)

#define EEPROM_SCORE_START_ADDR (0X0010)

#define EM_GAME_EEPROM_MAGIC_NUMBER ((uint32_t)0x656D6773)

typedef struct
{
	uint8_t best_goals;
	uint8_t best_difficulty;
	uint8_t reserved[2];
} em_game_score_record_t;

#ifdef __cplusplus
extern "C"
{
#endif

	extern void init_score_record(em_game_score_record_t* data);
	extern bool load_score_record(em_game_score_record_t* data);
	extern bool save_score_record(em_game_score_record_t* data);

#ifdef __cplusplus
}
#endif

#endif //__APP_EEPROM_H__
