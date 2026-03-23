#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"
#include "defs.h"

/* charger sub-states */
#define CHARGER_APPROACH    0
#define CHARGER_TELEGRAPH   1
#define CHARGER_DASHING     2
#define CHARGER_COOLDOWN    3

/* spinner sub-states */
#define SPINNER_WANDER_A    0  // wander, then shoot cross +
#define SPINNER_SHOOT_CROSS 1
#define SPINNER_WANDER_B    2  // wander, then shoot X
#define SPINNER_SHOOT_X     3

typedef struct 
{
    int HP;
    Entity *target;
    float lifetime;
    float time_alive;
    float speed;            // move speed from def
    char  behavior[32];     // "melee", "ranged", "charger", "caster"
    float shoot_cooldown;   // seconds until next shot (ranged/caster)
    float aoe_cooldown;     // seconds until next AOE drop (caster only)
    /* charger only */
    int   charge_state;     // CHARGER_* enum above
    float charge_timer;     // seconds remaining in current state
    GFC_Vector2D charge_dir;// locked direction for the dash
}MonsterData; 

/*
@brief spawn a new monster, loading its stats from enemies.def by type name
@param target the entity the monster will chase (player)
@param type name key from enemies.def e.g. "basic_melee"
@return pointer to entity or NULL on error
*/
Entity *monster_new(Entity *target, const char *type);

#endif
