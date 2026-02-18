#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

typedef struct 
{
    int HP;
    Entity *target;
    float lifetime;      // how long before it despawns
    float time_alive;    // how long it's been alive
}MonsterData; 

/*
/ @brief Creates a new player entity
/ @return Player entity pointer or NULL on error
*/
Entity *monster_new();

#endif
