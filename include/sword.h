#ifndef __SWORD_H__
#define __SWORD_H__

#include "entity.h"

/*
* @brief spawn a short-lived melee swing zone in front of the attacker.
*        Deals damage once to all enemies within an oriented rectangle on
*        the first frame, then shows a yellow rect outline that fades out.
* @param position world position of the attacker
* @param direction aim direction (will be normalized internally)
* @param damage damage to deal to each enemy hit
* @param faction faction of the attacker (opposite faction gets hit)
* @return pointer to the swing entity, or NULL on error
*/
Entity *sword_swing_new(GFC_Vector2D position, GFC_Vector2D direction, int damage, Uint8 faction);

#endif
/*eol@eof*/
