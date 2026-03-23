#include <string.h>

#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_color.h"

#include "defs.h"

static EnemyDef   _enemies[DEFS_MAX_ENEMIES];
static int        _enemy_count = 0;

static AbilityDef _abilities[DEFS_MAX_ABILITIES];
static int        _ability_count = 0;

static ClassDef   _classes[DEFS_MAX_CLASSES];
static int        _class_count = 0;

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
        str = sj_object_get_value_as_string(item, "color");
        if(str) strncpy(def->color_name, str, sizeof(GFC_TextLine) - 1);
        else    strncpy(def->color_name, "white", sizeof(GFC_TextLine) - 1);
    }
    sj_free(json);
    //slog("defs: loaded %d enemies from %s", _enemy_count, filename);
}

static int defs_ability_type_from_string(const char *str)
{
    if(!str) return ABILITY_TYPE_NONE;
    if(strcmp(str, "melee")        == 0) return ABILITY_TYPE_MELEE;
    if(strcmp(str, "projectile")   == 0) return ABILITY_TYPE_PROJECTILE;
    if(strcmp(str, "aoe")          == 0) return ABILITY_TYPE_AOE;
    if(strcmp(str, "dash")         == 0) return ABILITY_TYPE_DASH;
    if(strcmp(str, "blink")        == 0) return ABILITY_TYPE_BLINK;
    if(strcmp(str, "aoe_targeted") == 0) return ABILITY_TYPE_AOE_TARGETED;
    return ABILITY_TYPE_NONE;
}

static int defs_aoe_effect_from_string(const char *str)
{
    if(!str) return AOE_EFFECT_DAMAGE;
    if(strcmp(str, "damage")    == 0) return AOE_EFFECT_DAMAGE;
    if(strcmp(str, "heal")      == 0) return AOE_EFFECT_HEAL;
    if(strcmp(str, "burn")      == 0) return AOE_EFFECT_BURN;
    if(strcmp(str, "slow_zone") == 0) return AOE_EFFECT_SLOW_ZONE;
    return AOE_EFFECT_DAMAGE;
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
        str = sj_object_get_value_as_string(item, "type");
        if(str) def->type = defs_ability_type_from_string(str);
        sj_object_get_value_as_float(item, "aoe_radius",  &def->aoe_radius);
        str = sj_object_get_value_as_string(item, "aoe_effect");
        if(str) def->aoe_effect = defs_aoe_effect_from_string(str);
    }
    sj_free(json);
    //slog("defs: loaded %d abilities from %s", _ability_count, filename);
}

static void defs_load_classes(const char *filename)
{
    SJson *json, *list, *item, *arr;
    ClassDef *def;
    const char *str;
    int i, j, count, ab_count;

    json = sj_load(filename);
    if(!json){ slog("defs: could not load %s", filename); return; }

    list = sj_object_get_value(json, "classes");
    if(!list){ sj_free(json); return; }

    count = sj_array_get_count(list);
    for(i = 0; i < count && _class_count < DEFS_MAX_CLASSES; i++)
    {
        item = sj_array_get_nth(list, i);
        if(!item) continue;

        def = &_classes[_class_count++];
        memset(def, 0, sizeof(ClassDef));

        // defaults so missing fields don't zero out stats
        def->damage_mult  = 1.0f;
        def->ms           = 1.0f;
        def->attack_speed = 1.0f;
        def->crit_dmg     = 1.5f;
        def->knockback    = 1.0f;
        def->health       = 100;

        str = sj_object_get_value_as_string(item, "name");
        if(str) strncpy(def->name, str, sizeof(GFC_TextLine) - 1);

        str = sj_object_get_value_as_string(item, "displayName");
        if(str) strncpy(def->display_name, str, sizeof(GFC_TextLine) - 1);

        sj_object_get_value_as_int(item,   "health",            &def->health);
        sj_object_get_value_as_float(item, "damage",            &def->damage_mult);
        sj_object_get_value_as_float(item, "ms",                &def->ms);
        sj_object_get_value_as_int(item,   "armor",             &def->armor);
        sj_object_get_value_as_float(item, "attack_speed",      &def->attack_speed);
        sj_object_get_value_as_float(item, "crit",              &def->crit);
        sj_object_get_value_as_float(item, "crit_dmg",          &def->crit_dmg);
        sj_object_get_value_as_float(item, "lifesteal",         &def->lifesteal);
        sj_object_get_value_as_float(item, "knockback",         &def->knockback);

        // abilities array — up to 4 strings
        arr = sj_object_get_value(item, "abilities");
        if(arr)
        {
            ab_count = sj_array_get_count(arr);
            def->ability_count = (ab_count > 4) ? 4 : ab_count;
            for(j = 0; j < def->ability_count; j++)
            {
                str = sj_get_string_value(sj_array_get_nth(arr, j));
                if(str) strncpy(def->abilities[j], str, sizeof(GFC_TextLine) - 1);
            }
        }
    }
    sj_free(json);
    slog("defs: loaded %d classes from %s", _class_count, filename);
}

void defs_load_all()
{
    defs_load_enemies("defs/enemies.def");
    defs_load_abilities("defs/abilities.def");
    defs_load_classes("defs/classes.def");
}

void defs_free()
{
    _enemy_count   = 0;
    _ability_count = 0;
    _class_count   = 0;
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

ClassDef *defs_get_class(const char *name)
{
    int i;
    if(!name) return NULL;
    for(i = 0; i < _class_count; i++)
    {
        if(strcmp(_classes[i].name, name) == 0) return &_classes[i];
    }
    slog("defs: class not found: '%s'", name);
    return NULL;
}

GFC_Color defs_color_from_name(const char *name)
{
    if(!name || name[0] == '\0') return GFC_COLOR_WHITE;
    if(strcmp(name, "red")          == 0) return GFC_COLOR_RED;
    if(strcmp(name, "lightred")     == 0) return GFC_COLOR_LIGHTRED;
    if(strcmp(name, "darkred")      == 0) return GFC_COLOR_DARKRED;
    if(strcmp(name, "green")        == 0) return GFC_COLOR_GREEN;
    if(strcmp(name, "lightgreen")   == 0) return GFC_COLOR_LIGHTGREEN;
    if(strcmp(name, "darkgreen")    == 0) return GFC_COLOR_DARKGREEN;
    if(strcmp(name, "blue")         == 0) return GFC_COLOR_BLUE;
    if(strcmp(name, "lightblue")    == 0) return GFC_COLOR_LIGHTBLUE;
    if(strcmp(name, "darkblue")     == 0) return GFC_COLOR_DARKBLUE;
    if(strcmp(name, "yellow")       == 0) return GFC_COLOR_YELLOW;
    if(strcmp(name, "orange")       == 0) return GFC_COLOR_ORANGE;
    if(strcmp(name, "lightorange")  == 0) return GFC_COLOR_LIGHTORANGE;
    if(strcmp(name, "cyan")         == 0) return GFC_COLOR_CYAN;
    if(strcmp(name, "magenta")      == 0) return GFC_COLOR_MAGENTA;
    if(strcmp(name, "purple")       == 0) return GFC_COLOR_DARKMAGENTA;
    if(strcmp(name, "grey")         == 0) return GFC_COLOR_GREY;
    if(strcmp(name, "darkgrey")     == 0) return GFC_COLOR_DARKGREY;
    if(strcmp(name, "black")        == 0) return GFC_COLOR_BLACK;
    if(strcmp(name, "white")        == 0) return GFC_COLOR_WHITE;
    slog("defs_color_from_name: unknown color '%s', using white", name);
    return GFC_COLOR_WHITE;
}

/*eof@eof*/
