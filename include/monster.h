#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"
#include "defs.h"

typedef struct 
{
    int HP;
    Entity *target;
    float lifetime;
    float time_alive;
    float speed;        // move speed, loaded from def
}MonsterData; 

/*
@brief spawn a new monster, loading its stats from enemies.def by type name
@param target the entity the monster will chase (player)
@param type name key from enemies.def e.g. "basic_melee"
@return pointer to entity or NULL on error
*/
Entity *monster_new(Entity *target, const char *type);

#endif
