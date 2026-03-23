#include <math.h>
#include <string.h>

#include "simple_logger.h"

#include "monster.h"
#include "defs.h"
#include "projectile.h"
#include "aoe_spell.h"
#include "gfc_input.h"
#include "level.h"

void monster_think(Entity *self, float dt);
void monster_update(Entity *self, float dt);
void monster_free(Entity *self);

void monster_init_data(MonsterData *data, Entity *target)
{
    data->target        = target;
    data->lifetime      = 30.0f;
    data->time_alive    = 0;
    data->speed         = 1.0f;
    data->shoot_cooldown= 0;
    data->aoe_cooldown  = 3.0f; /* caster: delay before first AOE drop */
    data->charge_state  = CHARGER_APPROACH;
    data->charge_timer  = 0;
    data->charge_dir    = gfc_vector2d(0,0);
    strncpy(data->behavior, "melee", sizeof(data->behavior) - 1);
}

Entity *monster_new(Entity *target, const char *type)
{
    EnemyDef *def;
    GFC_Vector2D offset = gfc_vector2d(
        (gfc_random() * 2 * 600) - 600,
        (gfc_random() * 2 * 360) - 360
    );
    Entity *self;
    self = entity_new();
    if(!self)
    {
        slog("Error at Monster Entity Initialization");
        return NULL;
    }

    // load stats from def, fall back to defaults if not found
    def = defs_get_enemy(type);

    if(def)
    {
        self->sprite = gf2d_sprite_load_all(def->sprite, def->sprite_w, def->sprite_h, def->sprite_frames, 0);
        self->health     = def->health;
        self->max_health = def->health;
        self->damage     = def->damage;
        self->hit_radius = def->hit_radius;
        self->color      = defs_color_from_name(def->color_name);
    }
    else
    {
        // fallback so the game doesn't crash on a bad type name
        self->sprite = gf2d_sprite_load_all("images/space_bug_top.png", 128, 128, 17, 0);
        self->health = self->max_health = 30;
        self->damage     = 10;
        self->hit_radius = 12.0f;
    }

    self->frame = 0;
    gfc_vector2d_add(self->position, target->position, offset);
    self->rotation = 0;
    self->velocity = gfc_vector2d(0,0);
    self->faction  = 1;

    self->think  = monster_think;
    self->update = monster_update;
    self->free   = monster_free;

    self->data = gfc_allocate_array(sizeof(MonsterData), 1);
    if(!self->data) slog("error allocating monster data");
    monster_init_data((MonsterData*)self->data, target);

    // lifetime and speed from def
    if(def)
    {
        ((MonsterData*)self->data)->lifetime = def->lifetime;
        ((MonsterData*)self->data)->speed    = def->speed;
        strncpy(((MonsterData*)self->data)->behavior, def->behavior,
                sizeof(((MonsterData*)self->data)->behavior) - 1);
        // spinner needs a random wander direction from the start
        if(strcmp(((MonsterData*)self->data)->behavior, "spinner") == 0)
        {
            float angle = gfc_random() * 2.0f * 3.14159f;
            ((MonsterData*)self->data)->charge_dir   = gfc_vector2d(cosf(angle), sinf(angle));
            ((MonsterData*)self->data)->charge_timer = 1.5f + gfc_random() * 1.0f; // 1.5-2.5s wander
            ((MonsterData*)self->data)->charge_state = SPINNER_WANDER_A;
        }
    }

    return self;
}

void monster_think(Entity *self, float dt)
{
    MonsterData *data;
    float dx, dy, dist;
    GFC_Vector2D dir;
    AbilityDef *adef;

    if(!self) return;
    data = (MonsterData*)self->data;
    if(!data || !data->target) return;

    dx   = data->target->position.x - self->position.x;
    dy   = data->target->position.y - self->position.y;
    dist = sqrtf(dx*dx + dy*dy);

    // SPINNER — handled before face-player so it can spin freely
    if(strcmp(data->behavior, "spinner") == 0)
    {
        int k;
        float shoot_angles[4];
        self->rotation += 180.0f * dt; // 180 degrees/second constant spin

        if(data->charge_state == SPINNER_WANDER_A || data->charge_state == SPINNER_WANDER_B)
        {
            gfc_vector2d_scale(self->velocity, data->charge_dir, data->speed);
            data->charge_timer -= dt;
            if(data->charge_timer <= 0)
            {
                self->velocity = gfc_vector2d(0,0);
                if(data->charge_state == SPINNER_WANDER_A)
                {
                    shoot_angles[0] = 0.0f;   shoot_angles[1] = 90.0f;
                    shoot_angles[2] = 180.0f; shoot_angles[3] = 270.0f;
                    data->charge_state = SPINNER_SHOOT_CROSS;
                }
                else
                {
                    shoot_angles[0] = 45.0f;  shoot_angles[1] = 135.0f;
                    shoot_angles[2] = 225.0f; shoot_angles[3] = 315.0f;
                    data->charge_state = SPINNER_SHOOT_X;
                }
                adef = defs_get_ability("enemy_spinner_shot");
                for(k = 0; k < 4; k++)
                {
                    float rad = shoot_angles[k] * (3.14159f / 180.0f);
                    GFC_Vector2D shot_dir = gfc_vector2d(cosf(rad), sinf(rad));
                    projectile_new_from_ability(self->position, shot_dir, adef, self->faction);
                }
                data->charge_timer = 0.667f; // brief pause after firing (~40 frames)
            }
        }
        else // SPINNER_SHOOT_CROSS or SPINNER_SHOOT_X
        {
            self->velocity = gfc_vector2d(0,0);
            data->charge_timer -= dt;
            if(data->charge_timer <= 0)
            {
                float angle = gfc_random() * 2.0f * 3.14159f;
                data->charge_dir   = gfc_vector2d(cosf(angle), sinf(angle));
                data->charge_timer = 1.5f + gfc_random() * 1.0f; // 1.5-2.5s wander
                data->charge_state = (data->charge_state == SPINNER_SHOOT_CROSS)
                                     ? SPINNER_WANDER_B
                                     : SPINNER_WANDER_A;
            }
        }
        return;
    }

    // all other behaviors face the player
    self->rotation = atan2f(dy, dx) * (180.0f / 3.14159f);

    // --- MELEE: just chase ---
    if(strcmp(data->behavior, "melee") == 0)
    {
        dir = gfc_vector2d(dx, dy);
        gfc_vector2d_normalize(&dir);
        gfc_vector2d_scale(self->velocity, dir, data->speed);
        return;
    }

    // --- RANGED: keep ~180px distance, shoot on cooldown ---
    if(strcmp(data->behavior, "ranged") == 0)
    {
        float preferred = 180.0f;
        dir = gfc_vector2d(dx, dy);
        gfc_vector2d_normalize(&dir);
        if(dist > preferred + 20.0f)       // too far — close in
            gfc_vector2d_scale(self->velocity, dir, data->speed);
        else if(dist < preferred - 20.0f)  // too close — back off
            gfc_vector2d_scale(self->velocity, dir, -data->speed);
        else
            self->velocity = gfc_vector2d(0,0);

        if(data->shoot_cooldown > 0) { data->shoot_cooldown -= dt; }
        else if(dist < 400.0f)
        {
            float spread = 60.0f;
            GFC_Vector2D shot_dir = gfc_vector2d(
                dx + gfc_crandom() * spread,
                dy + gfc_crandom() * spread);
            adef = defs_get_ability("enemy_shot");
            projectile_new_from_ability(self->position, shot_dir, adef, self->faction);
            data->shoot_cooldown = adef ? adef->cooldown / 60.0f : 1.5f; // def cooldown is in frames
        }
        return;
    }

    if(strcmp(data->behavior, "charger") == 0)
    {
        if(data->charge_state == CHARGER_APPROACH)
        {
            dir = gfc_vector2d(dx, dy);
            gfc_vector2d_normalize(&dir);
            gfc_vector2d_scale(self->velocity, dir, data->speed);
            if(dist < 400.0f)
            {
                data->charge_state = CHARGER_TELEGRAPH;
                data->charge_timer = 2.0f; 
                data->charge_dir   = gfc_vector2d(dx, dy);
                gfc_vector2d_normalize(&data->charge_dir);
                self->velocity = gfc_vector2d(0,0);
            }
        }
        else if(data->charge_state == CHARGER_TELEGRAPH)
        {
            self->velocity = gfc_vector2d(0,0);
            // flash yellow/white every 0.1s to signal
            self->color = (((int)(data->charge_timer * 10)) % 2 == 0)
                ? GFC_COLOR_YELLOW : GFC_COLOR_WHITE;
            data->charge_timer -= dt;
            if(data->charge_timer <= 0)
            {
                data->charge_state = CHARGER_DASHING;
                data->charge_timer = 0.5f; // 0.5s dash
                self->color = GFC_COLOR_ORANGE;
            }
        }
        else if(data->charge_state == CHARGER_DASHING)
        {
            gfc_vector2d_scale(self->velocity, data->charge_dir, data->speed * 10.0f);
            data->charge_timer -= dt;
            if(data->charge_timer <= 0)
            {
                data->charge_state = CHARGER_COOLDOWN;
                data->charge_timer = 3.0f; // 3s cooldown before next charge
                self->velocity = gfc_vector2d(0,0);
                self->color = GFC_COLOR_ORANGE;
            }
        }
        else if(data->charge_state == CHARGER_COOLDOWN)
        {
            dir = gfc_vector2d(dx, dy);
            gfc_vector2d_normalize(&dir);
            gfc_vector2d_scale(self->velocity, dir, data->speed * 0.5f);
            data->charge_timer -= dt;
            if(data->charge_timer <= 0)
                data->charge_state = CHARGER_APPROACH;
        }
        return;
    }

    // --- CASTER: keep distance, lob slow heavy shot ---
    if(strcmp(data->behavior, "caster") == 0)
    {
        float preferred = 220.0f;
        dir = gfc_vector2d(dx, dy);
        gfc_vector2d_normalize(&dir);
        if(dist > preferred + 20.0f)
            gfc_vector2d_scale(self->velocity, dir, data->speed);
        else if(dist < preferred - 20.0f)
            gfc_vector2d_scale(self->velocity, dir, -data->speed);
        else
            self->velocity = gfc_vector2d(0,0);

        if(data->shoot_cooldown > 0) { data->shoot_cooldown -= dt; }
        else if(dist < 500.0f)
        {
            float spread = 35.0f;
            GFC_Vector2D shot_dir = gfc_vector2d(
                dx + gfc_crandom() * spread,
                dy + gfc_crandom() * spread);
            adef = defs_get_ability("enemy_aoe_shot");
            projectile_new_from_ability(self->position, shot_dir, adef, self->faction);
            data->shoot_cooldown = adef ? adef->cooldown / 60.0f : 2.5f;
        }
        if(data->aoe_cooldown > 0) { data->aoe_cooldown -= dt; }
        else if(dist < 500.0f)
        {
            GFC_Vector2D cast_pos;
            adef = defs_get_ability("enemy_caster_dot");
            cast_pos = gfc_vector2d(
                data->target->position.x + gfc_crandom() * 100.0f,
                data->target->position.y + gfc_crandom() * 100.0f);
            if(adef)
                aoe_spell_new(cast_pos, adef, self->faction, adef->damage);
            data->aoe_cooldown = adef ? adef->cooldown / 60.0f : 4.0f;
        }
        return;
    }

    // default fallback — plain melee
    dir = gfc_vector2d(dx, dy);
    gfc_vector2d_normalize(&dir);
    gfc_vector2d_scale(self->velocity, dir, data->speed);
}

void monster_update(Entity *self, float dt)
{
    MonsterData *data;

    if(!self)return;
    self->frame += 6.0f * dt; // 6 animation frames/second at any framerate
    if (self->frame >= 16) self->frame = 0;

    self->position.x += self->velocity.x * dt * 60.0f;
    self->position.y += self->velocity.y * dt * 60.0f;
    entity_resolve_tile_collision(self, gCurrentLevel, dt);

    data = (MonsterData*)self->data;
    if (data)
    {
        data->time_alive += dt;
        if (data->time_alive >= data->lifetime)
            self->_delete_me = 1;
    }
}

void monster_free(Entity *self)
{
    if(!self)return;

    if(self->data)
    {
        free(self->data);
        self->data = NULL;
    }
}
