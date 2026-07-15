#ifndef __EM_GAME_MATCH_H__
#define __EM_GAME_MATCH_H__

#define EM_GAME_MATCH_MISS_BASE_CHANCE (10)
#define EM_GAME_MATCH_MISS_FAST_BONUS (20)
#define EM_GAME_MATCH_MISS_MAX_CHANCE (30)
#define EM_GAME_MATCH_FAST_REACTION_INTERVAL (500)
#define EM_GAME_MATCH_RANDOM_FALLBACK (0xA341316CUL)

extern void em_game_match_handle(ak_msg_t* msg);

#endif //__EM_GAME_MATCH_H__
