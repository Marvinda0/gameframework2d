#include "simple_logger.h"

#include "projectile.h"
#include "level.h"

void projectile_think(Entity *self);
void projectile_update(Entity *self);
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
    gfc_vector2d_scale(self->velocity, direction, speed);

    self->faction = faction;
    self->is_projectile = 1;
    self->damage = damage;
    self->health = 1;
    self->hit_radius = 8.0f;

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

void projectile_think(Entity *self)
{
    if(!self) return;
    // nothing needed — velocity is set at spawn
}

void projectile_update(Entity *self)
{
    ProjectileData *data;
    if(!self) return;

    // move
    gfc_vector2d_add(self->position, self->position, self->velocity);

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
        data->time_alive += 0.016f;
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

/*eof@eof*/
