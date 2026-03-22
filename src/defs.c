#include <string.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "defs.h"

static EnemyDef   _enemies[DEFS_MAX_ENEMIES];
static int        _enemy_count = 0;

static AbilityDef _abilities[DEFS_MAX_ABILITIES];
static int        _ability_count = 0;

static void defs_load_enemies(const char *filename)
{
    SJson *json, *list, *item;
    EnemyDef *def;
    const char *str;
    int i, count;

    json = sj_load(filename);
    if(!json){ slog("defs: could not load %s", filename); return; }

    list = sj_object_get_value(json, "enemies");
    if(!list){ sj_free(json); return; }

    count = sj_array_get_count(list);
    for(i = 0; i < count && _enemy_count < DEFS_MAX_ENEMIES; i++)
    {
        item = sj_array_get_nth(list, i);
        if(!item) continue;

        def = &_enemies[_enemy_count++];
        memset(def, 0, sizeof(EnemyDef));

        // string fields
        str = sj_object_get_value_as_string(item, "name");
        if(str) strncpy(def->name, str, sizeof(GFC_TextLine) - 1);

        str = sj_object_get_value_as_string(item, "sprite");
        if(str) strncpy(def->sprite, str, sizeof(GFC_TextLine) - 1);

        str = sj_object_get_value_as_string(item, "behavior");
        if(str) strncpy(def->behavior, str, sizeof(GFC_TextLine) - 1);

        // numeric fields
        sj_object_get_value_as_int(item,   "sprite_w",      &def->sprite_w);
        sj_object_get_value_as_int(item,   "sprite_h",      &def->sprite_h);
        sj_object_get_value_as_int(item,   "sprite_frames", &def->sprite_frames);
        sj_object_get_value_as_int(item,   "health",        &def->health);
        sj_object_get_value_as_int(item,   "damage",        &def->damage);
        sj_object_get_value_as_float(item, "speed",         &def->speed);
        sj_object_get_value_as_float(item, "hit_radius",    &def->hit_radius);
        sj_object_get_value_as_float(item, "lifetime",      &def->lifetime);
    }
    sj_free(json);
    slog("defs: loaded %d enemies from %s", _enemy_count, filename);
}

static void defs_load_abilities(const char *filename)
{
    SJson *json, *list, *item;
    AbilityDef *def;
    const char *str;
    int i, count;

    json = sj_load(filename);
    if(!json){ slog("defs: could not load %s", filename); return; }

    list = sj_object_get_value(json, "abilities");
    if(!list){ sj_free(json); return; }

    count = sj_array_get_count(list);
    for(i = 0; i < count && _ability_count < DEFS_MAX_ABILITIES; i++)
    {
        item = sj_array_get_nth(list, i);
        if(!item) continue;

        def = &_abilities[_ability_count++];
        memset(def, 0, sizeof(AbilityDef));

        str = sj_object_get_value_as_string(item, "name");
        if(str) strncpy(def->name, str, sizeof(GFC_TextLine) - 1);

        str = sj_object_get_value_as_string(item, "sprite");
        if(str) strncpy(def->sprite, str, sizeof(GFC_TextLine) - 1);

        sj_object_get_value_as_int(item,   "sprite_w",      &def->sprite_w);
        sj_object_get_value_as_int(item,   "sprite_h",      &def->sprite_h);
        sj_object_get_value_as_int(item,   "sprite_frames", &def->sprite_frames);
        sj_object_get_value_as_int(item,   "damage",        &def->damage);
        sj_object_get_value_as_float(item, "speed",         &def->speed);
        sj_object_get_value_as_float(item, "cooldown",      &def->cooldown);
        sj_object_get_value_as_float(item, "hit_radius",    &def->hit_radius);
        sj_object_get_value_as_float(item, "lifetime",      &def->lifetime);
        sj_object_get_value_as_int(item,   "is_projectile", &def->is_projectile);
        sj_object_get_value_as_float(item, "aoe_radius",    &def->aoe_radius);
    }
    sj_free(json);
    slog("defs: loaded %d abilities from %s", _ability_count, filename);
}

void defs_load_all()
{
    defs_load_enemies("defs/enemies.def");
    defs_load_abilities("defs/abilities.def");
}

void defs_free()
{
    _enemy_count   = 0;
    _ability_count = 0;
}

EnemyDef *defs_get_enemy(const char *name)
{
    int i;
    if(!name) return NULL;
    for(i = 0; i < _enemy_count; i++)
    {
        if(strcmp(_enemies[i].name, name) == 0) return &_enemies[i];
    }
    slog("defs: enemy not found: '%s'", name);
    return NULL;
}

AbilityDef *defs_get_ability(const char *name)
{
    int i;
    if(!name) return NULL;
    for(i = 0; i < _ability_count; i++)
    {
        if(strcmp(_abilities[i].name, name) == 0) return &_abilities[i];
    }
    slog("defs: ability not found: '%s'", name);
    return NULL;
}

/*eof@eof*/
