#include "app_eeprom.h"
#include "eeprom.h"

#include "game_eleven_meter/em_game_match.h"

typedef struct __attribute__((packed))
{
	uint32_t magic_number;
	em_game_score_record_t data;
	uint8_t check_sum;
} em_game_score_eeprom_t;

#define EM_GAME_SCORE_CHECKSUM_SIZE \
	(sizeof(uint32_t) + sizeof(em_game_score_record_t))
#define EM_GAME_SCORE_EEPROM_SIZE (9)

static_assert(sizeof(em_game_score_eeprom_t) == EM_GAME_SCORE_EEPROM_SIZE,
              "em_game score EEPROM record must be 9 bytes");

static uint8_t em_game_eeprom_checksum(const uint8_t* data, uint32_t size)
{
	uint8_t check_sum = 0;

	for (uint32_t i = 0; i < size; i++)
	{
		check_sum += data[i];
	}

	return check_sum;
}

static void
em_game_eeprom_update_checksum(em_game_score_eeprom_t* eeprom_data)
{
	eeprom_data->magic_number = EM_GAME_EEPROM_MAGIC_NUMBER;
	eeprom_data->check_sum = em_game_eeprom_checksum((const uint8_t*)eeprom_data,
	                                                 EM_GAME_SCORE_CHECKSUM_SIZE);
}

static bool em_game_eeprom_is_valid(const em_game_score_eeprom_t* eeprom_data)
{
	return (eeprom_data->magic_number == EM_GAME_EEPROM_MAGIC_NUMBER) &&
	       (eeprom_data->check_sum ==
	        em_game_eeprom_checksum((const uint8_t*)eeprom_data,
	                                EM_GAME_SCORE_CHECKSUM_SIZE));
}

void init_score_record(em_game_score_record_t* data)
{
	data->best_goals = 0;
	data->best_difficulty = EM_GAME_MATCH_DIFFICULTY_EASY;
	data->reserved[0] = 0;
	data->reserved[1] = 0;
}

bool load_score_record(em_game_score_record_t* data)
{
	em_game_score_eeprom_t eeprom_data;
	uint8_t ret = eeprom_read(EEPROM_SCORE_START_ADDR, (uint8_t*)&eeprom_data,
	                          sizeof(eeprom_data));

	if ((ret == EEPROM_DRIVER_OK) && em_game_eeprom_is_valid(&eeprom_data))
	{
		*data = eeprom_data.data;
		return true;
	}

	init_score_record(data);
	return false;
}

bool save_score_record(em_game_score_record_t* data)
{
	em_game_score_eeprom_t eeprom_data;

	eeprom_data.data = *data;
	em_game_eeprom_update_checksum(&eeprom_data);

	return eeprom_write(EEPROM_SCORE_START_ADDR, (uint8_t*)&eeprom_data,
	                    sizeof(eeprom_data)) == EEPROM_DRIVER_OK;
}
