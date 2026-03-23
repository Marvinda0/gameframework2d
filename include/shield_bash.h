#ifndef __SHIELD_BASH_H__
#define __SHIELD_BASH_H__

#include "entity.h"
#include "gfc_vector.h"

/*
 * @brief spawn a shield bash hit zone in front of the attacker.
 *        Deals damage and knocks back enemies in a square area.
 *        TODO(def): should accept AbilityDef* and read damage/cooldown/size from "shield_bash" entry
 * @param position world-space position of the attacker
 * @param direction normalized aim direction
 * @param damage flat damage to deal (TODO: scale with armor stat when implemented)
 * @param faction attacking faction (enemies of this faction get hit + knocked back)
 * @return pointer to the bash entity, or NULL on error
 */
Entity *shield_bash_new(GFC_Vector2D position, GFC_Vector2D direction, int damage, Uint8 faction);

#endif
/*eol@eof*/
