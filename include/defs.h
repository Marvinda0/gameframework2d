#ifndef __DEFS_H__
#define __DEFS_H__

#include "gfc_text.h"
#include "gfc_color.h"

/* @brief look up a GFC_Color by name string. Returns GFC_COLOR_WHITE if unknown. */
GFC_Color defs_color_from_name(const char *name);

#define DEFS_MAX_ENEMIES   32
#define DEFS_MAX_ABILITIES 32
#define DEFS_MAX_CLASSES    8

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
    GFC_TextLine color_name; 
} EnemyDef;

typedef enum
{
    ABILITY_TYPE_NONE         = 0,
    ABILITY_TYPE_MELEE        = 1,  // short-lived OBB hitbox (sword, bash)
    ABILITY_TYPE_PROJECTILE   = 2,  // moving projectile entity toward cursor
    ABILITY_TYPE_AOE          = 3,  // instant AOE circle at cast position
    ABILITY_TYPE_DASH         = 4,  // mutates player velocity directly (charge, roll)
    ABILITY_TYPE_BLINK        = 5,  // instant position teleport toward cursor + iframes
    ABILITY_TYPE_AOE_TARGETED = 6,  // two-press: first arms targeting preview, second casts
} AbilityType;

typedef enum
{
    AOE_EFFECT_DAMAGE    = 0,  // deals damage 
    AOE_EFFECT_HEAL      = 1,  // heals allies 
    AOE_EFFECT_BURN      = 2,  // fire damage 
    AOE_EFFECT_SLOW_ZONE = 3,  // slows enemies 
} AoeEffect;

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
    int   type;         // AbilityType — determines how player fires this ability
    float aoe_radius;
    int   aoe_effect;   // AoeEffect — tick behavior + visual color of the AOE ring
} AbilityDef;

/* @brief load enemies.def and abilities.def — call once at startup */
void defs_load_all();

/* @brief free all loaded def data */
void defs_free();

/* @brief look up an enemy def by name, returns NULL if not found */
EnemyDef *defs_get_enemy(const char *name);

/* @brief look up an ability def by name, returns NULL if not found */
AbilityDef *defs_get_ability(const char *name);

// loaded from classes.def
typedef struct
{
    GFC_TextLine name;          // "knight", "wizard", "ranger"
    GFC_TextLine display_name;  // "Knight", etc.
    int   health;               // base HP pool
    float damage_mult;          // multiplier on all outgoing damage (1.0 = base)
    float ms;                   // move speed multiplier (1.0 = base)
    int   armor;                // flat damage reduction per hit received
    float attack_speed;         // cooldown divisor (2.0 = half CDs, double attack rate)
    float crit;                 // crit chance 0.0–1.0
    float crit_dmg;             // crit damage multiplier (1.5 = +50% on crit)
    float lifesteal;            // fraction of damage dealt healed (0.1 = 10%)
    float knockback;            // multiplier on outgoing knockback
    GFC_TextLine abilities[4];  // ability names for this class
    int   ability_count;
} ClassDef;

/* @brief load classes.def — call once at startup (called inside defs_load_all) */
ClassDef *defs_get_class(const char *name);

#endif
