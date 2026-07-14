#include "fsm.h"
#include "port.h"
#include "message.h"

#include "em_game_match.h"
#include "em_game_match_state.h"

static em_game_match_state_t s_match;
static bool s_match_initialized = false;

void em_game_match_handle(ak_msg_t* msg)
{
	if (!s_match_initialized)
	{
		em_game_match_state_init(&s_match);
		s_match_initialized = true;
	}

	(void)msg;
}
