#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

typedef struct 
{
    int HP;
    Entity *target;
}MonsterData; 

/*
/ @brief Creates a new player entity
/ @return Player entity pointer or NULL on error
*/
Entity *monster_new();

#endif
