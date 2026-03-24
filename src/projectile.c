#include "simple_logger.h"

#include "projectile.h"
#include "level.h"

void projectile_think(Entity *self, float dt);
void projectile_update(Entity *self, float dt);
void projectile_free(Entity *self);

Entity *projectile_new(GFC_Vector2D position, GFC_Vector2D direction, float speed, int damage, Uint8 faction)
{
    Entity *self;
    ProjectileData *data;

    self = entity_new();
    if(!self)
    {
        slog("Error at Projectile Entity Initialization");
        return NULL;
    }

    self->sprite = gf2d_sprite_load_all("images/pointer.png", 32, 32, 16, 0); // placeholder sprite
    self->frame = 0;
    self->position = position;

    gfc_vector2d_normalize(&direction);
    gfc_vector2d_scale(self->velocity, direction, speed); // velocity is fixed at spawn, no acceleration
    self->rotation = atan2f(direction.y, direction.x) * (180.0f / 3.14159f); // face travel direction

    self->faction = faction;    // same faction as caster, so it won't hurt friendlies
    self->is_projectile = 1;    // flag tells collision system to despawn this on hit
    self->damage = damage;
    self->health = 1;
    self->hit_radius = 8.0f;    // small circle, tweak per ability later

    self->think = projectile_think;
    self->update = projectile_update;
    self->free = projectile_free;

    data = gfc_allocate_array(sizeof(ProjectileData), 1);
    if(!data) slog("error allocating projectile data");
    data->lifetime = 3.0f;
    data->time_alive = 0;
    self->data = data;

    return self;
}

void projectile_think(Entity *self, float dt)
{
    if(!self) return;
    // nothing needed — velocity is set at spawn
    (void)dt;
}

void projectile_update(Entity *self, float dt)
{
    ProjectileData *data;
    if(!self) return;

    // move
    self->position.x += self->velocity.x * dt * 60.0f;
    self->position.y += self->velocity.y * dt * 60.0f;

    // despawn on wall hit
    if(gCurrentLevel && level_get_tile_at(gCurrentLevel, self->position.x, self->position.y))
    {
        self->_delete_me = 1;
        return;
    }

    // lifetime
    data = (ProjectileData*)self->data;
    if(data)
    {
        data->time_alive += dt;
        if(data->time_alive >= data->lifetime)
            self->_delete_me = 1;
    }
}

void projectile_free(Entity *self)
{
    if(!self) return;
    if(self->data)
    {
        free(self->data);
        self->data = NULL;
    }
}

// spawn a projectile from an AbilityDef — sprite, damage, speed, lifetime all from JSON
Entity *projectile_new_from_ability(GFC_Vector2D position, GFC_Vector2D direction, AbilityDef *def, Uint8 faction, int override_damage, float crit_chance, float crit_dmg_mult, float lifesteal)
{
    Entity *self;
    ProjectileData *data;

    if(!def) return projectile_new(position, direction, 8.0f, 10, faction);

    self = entity_new();
    if(!self) return NULL;

    self->sprite = gf2d_sprite_load_all(def->sprite, def->sprite_w, def->sprite_h, def->sprite_frames, 0);
    self->frame    = 0;
    self->position = position;

    gfc_vector2d_normalize(&direction);
    gfc_vector2d_scale(self->velocity, direction, def->speed);
    self->rotation = atan2f(direction.y, direction.x) * (180.0f / 3.14159f);

    self->faction       = faction;
    self->is_projectile = 1;
    self->damage        = (override_damage >= 0) ? override_damage : def->damage;
    self->health        = 1;
    self->hit_radius    = def->hit_radius;
    self->crit_chance   = crit_chance;
    self->crit_dmg_mult = crit_dmg_mult;
    self->lifesteal     = lifesteal;

    self->think  = projectile_think;
    self->update = projectile_update;
    self->free   = projectile_free;

    data = gfc_allocate_array(sizeof(ProjectileData), 1);
    if(!data) slog("error allocating projectile data");
    data->lifetime   = def->lifetime;
    data->time_alive = 0;
    self->data = data;

    return self;
}

/*eof@eof*/
