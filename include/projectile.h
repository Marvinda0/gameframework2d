#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"
#include "defs.h"

typedef struct
{
    float lifetime;
    float time_alive;
} ProjectileData;

/*
@brief spawn a projectile using raw values (generic use)
*/
Entity *projectile_new(GFC_Vector2D position, GFC_Vector2D direction, float speed, int damage, Uint8 faction);

/*
@brief spawn a projectile using an AbilityDef — all values come from the def file
*/
Entity *projectile_new_from_ability(GFC_Vector2D position, GFC_Vector2D direction, AbilityDef *def, Uint8 faction);

#endif
