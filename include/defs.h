#ifndef __DEFS_H__
#define __DEFS_H__

#include "gfc_text.h"

#define DEFS_MAX_ENEMIES   32
#define DEFS_MAX_ABILITIES 32

// loaded from enemies.def
typedef struct
{
    GFC_TextLine name;
    GFC_TextLine sprite;
    int   sprite_w, sprite_h, sprite_frames;
    int   health;
    int   damage;
    float speed;
    float hit_radius;
    float lifetime;
    GFC_TextLine behavior; // "melee", "ranged", "charger", "caster"
} EnemyDef;

// loaded from abilities.def
typedef struct
{
    GFC_TextLine name;
    GFC_TextLine sprite;
    int   sprite_w, sprite_h, sprite_frames;
    int   damage;
    float speed;
    float cooldown;     // frames
    float hit_radius;
    float lifetime;     // seconds
    int   is_projectile;
    float aoe_radius;
} AbilityDef;

/* @brief load enemies.def and abilities.def — call once at startup */
void defs_load_all();

/* @brief free all loaded def data */
void defs_free();

/* @brief look up an enemy def by name, returns NULL if not found */
EnemyDef *defs_get_enemy(const char *name);

/* @brief look up an ability def by name, returns NULL if not found */
AbilityDef *defs_get_ability(const char *name);

#endif
