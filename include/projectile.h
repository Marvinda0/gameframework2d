#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

typedef struct
{
    float lifetime;
    float time_alive;
} ProjectileData;

/*
@brief spawn a new projectile entity
@param position world-space spawn position
@param direction unit vector for travel direction (will be normalized)
@param speed pixels per frame
@param damage damage dealt on hit
@param faction 0 = player projectile (hits enemies), 1 = enemy projectile (hits player)
@return pointer to entity or NULL on error
*/
Entity *projectile_new(GFC_Vector2D position, GFC_Vector2D direction, float speed, int damage, Uint8 faction);

#endif
