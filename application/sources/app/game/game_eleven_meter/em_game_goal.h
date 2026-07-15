#ifndef __EM_GAME_GOAL_H__
#define __EM_GAME_GOAL_H__

typedef struct
{
	uint8_t ball_direction;
	uint8_t keeper_direction;
} em_game_goal_hit_t;

extern void em_game_goal_handle(ak_msg_t* msg);

#endif //__EM_GAME_GOAL_H__
