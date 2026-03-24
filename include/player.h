#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "defs.h"

// charge ability states
#define CHARGE_IDLE    0
#define CHARGE_WINDUP  1
#define CHARGE_DASHING 2

// XP system
#define XP_COST_PER_UPGRADE   5
// stat upgrade caps
#define UPGRADE_CAP_MAX_HP    2000
#define UPGRADE_CAP_ARMOR     200
#define UPGRADE_CAP_DMG       3.0f
#define UPGRADE_CAP_CRIT      1.0f
#define UPGRADE_CAP_MS        12.0f
#define UPGRADE_CAP_ASPD      10.0f
#define UPGRADE_CAP_LS        1.5f

typedef struct
{
    GFC_TextLine className;
    float damage_mult;      // class damage multiplier 
    float crit;             // crit chance 0.0–1.0
    float crit_dmg;         // crit multiplier 
    float lifesteal;        // fraction of damage dealt that heals the player
    float ms;               // move speed (base * class ms multiplier)
    float armor;            // kept here for display; actual armor lives on Entity
    float attack_area_size; // reserved for future (scales hitbox size)
    float attack_speed;     // cooldown divisor — 2.0 = half CDs = double rate
    float knockback;        // outgoing knockback multiplier


    AbilityDef *ability_defs[4]; 
    float cooldowns[4];

    // charge / dash state machine
    int          charge_state;  // CHARGE_IDLE / WINDUP / DASHING
    float        charge_timer;  // seconds remaining in current phase
    GFC_Vector2D charge_dir;    // locked aim direction set at windup start

    // targeted AOE state (blizzard two-press)
    int          targeting_active; // 1 = waiting for second press to cast
    int          targeting_slot;   // which ability_defs slot is being targeted
    GFC_Vector2D target_pos;       // current world-space aim position

    // barrage burst state
    int          burst_remaining;  // arrows still to fire in current burst
    float        burst_timer;      // seconds until next arrow fires
    GFC_Vector2D burst_dir;        // direction locked when burst was triggered

    int          xp;  // accumulated experience points

    // permanent profile bonuses (applied at class-switch, updated on purchase)
    float        pickup_range;  // item pickup radius in pixels (base 30 + upgrades)
    int          has_revive;    // 1 = second-chance revive available this run
} PlayerData;


/*
/ @brief Creates a new player entity starting as knight
/ @return Player entity pointer or NULL on error
*/
Entity *player_new();

/*
/ @brief Switch the player's active class — reloads all stats and abilities from def
/ @param self  Player entity
/ @param className  Class name matching an entry in classes.def ("knight", "wizard", "ranger")
*/
void player_switch_class(Entity *self, const char *className);

/*
/ @brief Custom draw function — draws sprite + targeting preview ring when active
*/
void player_draw(Entity *self);

/*
/ @brief Return a vector with x and y of the palyer
/ @return Return a vector with x and y of the palyer
*/
GFC_Vector2D player_get_pos();

// add XP to the player (called from monster_free on kill)
void player_give_xp(Entity *player, int amount);

// spend XP_COST_PER_UPGRADE xp to upgrade one stat
// stat_index: 0=maxHP 1=armor 2=damage 3=crit 4=ms 5=aspd 6=lifesteal
void player_try_upgrade(Entity *player, int stat_index);

/*
/ @brief Apply permanent profile upgrade bonuses on top of current class base stats.
/        Called automatically at the end of player_switch_class.
/        Also call after purchasing a permanent upgrade to apply the new level.
*/
void player_apply_profile(Entity *self);

#endif