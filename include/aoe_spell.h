#ifndef __AOE_SPELL_H__
#define __AOE_SPELL_H__

#include "entity.h"
#include "defs.h"

/*
@brief Spawn a persistent AOE circle that deals damage every 0.5s until it expires.
       Used for player spells (blizzard) and caster enemy abilities.
@param position  world-space center of the AOE
@param def       ability def — reads aoe_radius and lifetime
@param faction   attacking faction (0=player hits enemies, 1=enemy hits player)
@param damage    pre-calculated damage per tick (already through player_calc_dmg)
@return pointer to the AOE entity, or NULL on error
*/
Entity *aoe_spell_new(GFC_Vector2D position, AbilityDef *def, Uint8 faction, int damage);

#endif
