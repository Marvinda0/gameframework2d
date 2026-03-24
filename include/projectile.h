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
       pass override_damage >= 0 to replace def->damage (e.g. for damage scaling)
       pass crit_chance > 0 so the collision system rolls crit at hit time
*/
Entity *projectile_new_from_ability(GFC_Vector2D position, GFC_Vector2D direction, AbilityDef *def, Uint8 faction, int override_damage, float crit_chance, float crit_dmg_mult, float lifesteal);

#endif
