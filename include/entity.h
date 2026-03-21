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
    int invincible_frames;  // i-frame countdown (ticks down each frame)
    Uint8 faction;          // 0 = player/friendly, 1 = enemy
    Uint8 is_projectile;    // despawns on hitting opposite faction entity
    float hit_radius;       // collision circle radius in pixels

	void (*think)(struct Entity_S* self);
	void (*update)(struct Entity_S *self);
	void (*free)(struct Entity_S* self);
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
void entity_system_think();

/*
* @brief run the update functions of all the entities 
*/
void entity_system_update();

/*
* @brief run the draw functions of all the entities 
*/
void entity_system_draw();

/*
@brief checks 4 points around entity, pushes back out of walls undoing movement actions before draw call
@param self entity to chekc collisions with environment
@param Level current game world 
*/
void entity_resolve_tile_collision(Entity *self, Level *level);

/*
@brief check all active entities against each other, apply damage on faction mismatch, despawn projectiles on hit
*/
void entity_system_check_collisions();

#endif