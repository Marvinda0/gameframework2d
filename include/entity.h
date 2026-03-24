#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gf2d_sprite.h"
#include "gfc_types.h"
#include "level.h"


typedef struct Entity_S
{
    Uint8 _inuse;
    Uint8 _delete_me;
    GFC_TextLine name;
    GFC_Vector2D position;
    GFC_Vector2D velocity;
    float rotation;
    GFC_Vector2D scale;
    GFC_Color color;
    Sprite *sprite;
    float frame;

    // combat
    int health;
    int max_health;
    int damage;             // contact/projectile damage dealt
    int armor;              // flat damage reduction per hit received (0 = no reduction)
    float invincible_timer; // i-frame countdown in SECONDS (ticks down each frame)
    Uint8 faction;          // 0 = player/friendly, 1 = enemy
    Uint8 is_projectile;    // despawns on hitting opposite faction entity
    float hit_radius;       // collision circle radius in pixels
    float crit_chance;      // 0.0-1.0 probability of a crit (0 = never crits)
    float crit_dmg_mult;    // damage multiplier on crit (e.g. 2.0 = double damage)
    float lifesteal;        // fraction of damage dealt healed back to this entity (0 = none)

	void (*think)(struct Entity_S* self, float dt);
	void (*update)(struct Entity_S *self, float dt);
	void (*free)(struct Entity_S* self);
    void (*draw)(struct Entity_S *self); 
    void *data;
} Entity;

/*
* @brief this initializes the entity system and queues up cleanup on exit
* @param max the maximum number of entities the entity system will hold
*/
void entity_system_init(Uint32 max);

/*
* @brief cleanup all active entities
*/
void entity_clear_all();

/*
* @brief allocate an empty entity sruct for use
* @return NULL on error or pointer to blank entity
*/
Entity *entity_new();

/*
* @brief free an entity from memory so it can be used for another entity
* @param self pointer to entity to be freed
*/
void entity_free(Entity *self);

/*
* @brief run the think functions of all the entities 
*/
void entity_system_think(float dt);

/*
* @brief run the update functions of all the entities 
*/
void entity_system_update(float dt);

/*
* @brief run the draw functions of all the entities 
*/
void entity_system_draw();

/*
@brief checks 4 points around entity, pushes back out of walls undoing movement actions before draw call
@param self entity to chekc collisions with environment
@param Level current game world 
@param dt delta time in seconds
*/
void entity_resolve_tile_collision(Entity *self, Level *level, float dt);

/*
@brief check all active entities against each other, apply damage on faction mismatch, despawn projectiles on hit
@param dt delta time in seconds
*/
void entity_system_check_collisions(float dt);

/*
@brief deal damage once to all active entities of the opposing faction whose hitbox overlaps
       an oriented rectangle. Used for melee abilities.
@param center world-space center of the rectangle
@param dir normalized forward direction of the rectangle
@param half_reach half-length along dir
@param half_width half-length perpendicular to dir
@param damage damage to apply per hit
@param attacking_faction entities of the OPPOSITE faction get hit
@param iframes invincibility frames to set on hit targets
*/
void entity_damage_in_rect(GFC_Vector2D center, GFC_Vector2D dir,
                           float half_reach, float half_width,
                           int damage, Uint8 attacking_faction, float iframes,
                           float crit_chance, float crit_dmg_mult, float lifesteal);

/*
@brief returns accumulated lifesteal HP since the last call (may be 0).
       Apply to the player entity in game.c once per frame.
*/
int entity_consume_lifesteal_heal(void);

/*
@brief returns 1 and clears the internal flag if a crit landed since the last call.
       Call once per frame in game.c to fire hud_trigger_crit_flash().
*/
int entity_consume_crit(void);

/*
@brief deal damage once to all active entities of the opposing faction within a circle.
       Used for AOE abilities (blizzard, caster spells).
@param center world-space center of the circle
@param radius radius in pixels
@param damage damage to apply per hit
@param attacking_faction entities of the OPPOSITE faction get hit
@param iframes invincibility seconds to set on hit targets
*/
void entity_damage_in_circle(GFC_Vector2D center, float radius,
                             int damage, Uint8 attacking_faction, float iframes);

/** heals all in-use entities of the given faction within the circle, capped at max_health */
void entity_heal_in_circle(GFC_Vector2D center, float radius,
                           int heal_amount, Uint8 faction);

/** burn damage: bypasses armor and ignores iframes so every tick always lands */
void entity_burn_in_circle(GFC_Vector2D center, float radius,
                           int damage, Uint8 attacking_faction);

/**
 * @brief heal all in-use entities of the given faction within a circle
 * @param center      world-space center of the area
 * @param radius      radius in pixels
 * @param heal_amount HP restored per call (capped at max_health)
 * @param faction     only entities matching this faction are healed
 */
void entity_heal_in_circle(GFC_Vector2D center, float radius,
                           int heal_amount, Uint8 faction);

/*
@brief push all active enemies of the opposing faction that overlap an oriented rectangle
       directly along the forward direction by 'force' pixels. Used for knockback effects.
@param center world-space center of the rectangle
@param dir normalized forward direction
@param half_reach half-length along dir
@param half_width half-length perpendicular to dir
@param force pixels to push each hit enemy along dir
@param attacking_faction entities of the OPPOSITE faction get knocked back
*/
void entity_knockback_in_rect(GFC_Vector2D center, GFC_Vector2D dir,
                              float half_reach, float half_width,
                              float force, Uint8 attacking_faction);

#endif