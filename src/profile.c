#include <string.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "profile.h"

#define PROFILE_FILE "profile.cfg"

/* max levels per upgrade */
static const int _MAX[PERM_COUNT]  = { 5,   3,   5,   3,   3 };
/* gold cost per purchase */
static const int _COST[PERM_COUNT] = { 100, 100, 150, 200, 500 };

typedef struct
{
    int gold;
    int levels[PERM_COUNT];
} Profile;

static Profile _p = {0};

/* ---- public helpers ---------------------------------------------------- */

int profile_upgrade_max(int idx)
{
    if(idx < 0 || idx >= PERM_COUNT) return 0;
    return _MAX[idx];
}

int profile_upgrade_cost(int idx)
{
    if(idx < 0 || idx >= PERM_COUNT) return 0;
    return _COST[idx];
}

/* ---- save / load -------------------------------------------------------- */

void profile_save(void)
{
    SJson *json;
    json = sj_object_new();
    if(!json) { slog("profile_save: sj_object_new failed"); return; }
    sj_object_insert(json, "gold",            sj_new_int(_p.gold));
    sj_object_insert(json, "speed_level",     sj_new_int(_p.levels[PERM_SPEED]));
    sj_object_insert(json, "lifesteal_level", sj_new_int(_p.levels[PERM_LIFESTEAL]));
    sj_object_insert(json, "hp_level",        sj_new_int(_p.levels[PERM_HP]));
    sj_object_insert(json, "xp_level",        sj_new_int(_p.levels[PERM_XP_BONUS]));
    sj_object_insert(json, "revive",          sj_new_int(_p.levels[PERM_REVIVE]));
    sj_save(json, PROFILE_FILE);
    sj_free(json);
}

void profile_load(void)
{
    SJson *json;
    memset(&_p, 0, sizeof(Profile));
    json = sj_load(PROFILE_FILE);
    if(!json)
    {
        slog("profile: no save found (%s), starting fresh", PROFILE_FILE);
        return;
    }
    sj_object_get_value_as_int(json, "gold",            &_p.gold);
    sj_object_get_value_as_int(json, "speed_level",     &_p.levels[PERM_SPEED]);
    sj_object_get_value_as_int(json, "lifesteal_level", &_p.levels[PERM_LIFESTEAL]);
    sj_object_get_value_as_int(json, "hp_level",        &_p.levels[PERM_HP]);
    sj_object_get_value_as_int(json, "xp_level",        &_p.levels[PERM_XP_BONUS]);
    sj_object_get_value_as_int(json, "revive",          &_p.levels[PERM_REVIVE]);
    sj_free(json);
    slog("profile: loaded  gold=%d  speed=%d  lifesteal=%d  hp=%d  xp=%d  revive=%d",
         _p.gold,
         _p.levels[PERM_SPEED],  _p.levels[PERM_LIFESTEAL],
         _p.levels[PERM_HP],     _p.levels[PERM_XP_BONUS],
         _p.levels[PERM_REVIVE]);
}

/* ---- gold --------------------------------------------------------------- */

int profile_get_gold(void) { return _p.gold; }

void profile_add_gold(int amount)
{
    _p.gold += amount;
    if(_p.gold < 0) _p.gold = 0;
    profile_save();
}

/* ---- upgrades ----------------------------------------------------------- */

int profile_get_upgrade(int idx)
{
    if(idx < 0 || idx >= PERM_COUNT) return 0;
    return _p.levels[idx];
}

int profile_buy_upgrade(int idx)
{
    if(idx < 0 || idx >= PERM_COUNT)        return 0;
    if(_p.levels[idx] >= _MAX[idx])          return 0; /* already maxed */
    if(_p.gold < _COST[idx])                 return 0; /* insufficient gold */
    _p.gold -= _COST[idx];
    _p.levels[idx]++;
    profile_save();
    slog("profile: bought upgrade %d  new_lv=%d  gold_remaining=%d",
         idx, _p.levels[idx], _p.gold);
    return 1;
}
