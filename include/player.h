#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct
{
    int HP;
}Player;


/*
/ @brief Creates a new player entity
/ @return Player entity pointer or NULL on error
*/
Entity *player_new();

/*
/ @brief Return a vector with x and y of the palyer
/ @return Return a vector with x and y of the palyer
*/
GFC_Vector2D player_get_pos();

#endif