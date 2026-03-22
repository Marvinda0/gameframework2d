#include "simple_logger.h"

#include "monster.h"
#include "defs.h"
#include "gfc_input.h"
#include "level.h"

void monster_think(Entity *self);
void monster_update(Entity *self);
void monster_free(Entity *self);

void monster_init_data(MonsterData *data, Entity *target)
{
    data->target     = target;
    data->lifetime   = 30.0f;
    data->time_alive = 0;
    data->speed      = 1.0f; // default, overridden by def
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
    }
    else
    {
        // fallback so the game doesn't crash on a bad type name
        self->sprite = gf2d_sprite_load_all("images/space_bug_top.png", 128, 128, 17, 0);
        self->health = self->max_health = 30;
        self->damage     = 10;
        self->hit_radius = 48.0f;
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
    }

    return self;
}

void monster_think(Entity *self)
{
    MonsterData *data;
    float dx, dy;
    if(!self)return; 
    
    data = (MonsterData*)self->data;
    if(!data || !data->target) return;
    dx = data->target->position.x - self->position.x;
    dy = data->target->position.y - self->position.y;

    float angle = atan2(dy,dx);
    self->rotation = angle * (180.0 / 3.14159);

    // move toward player
    GFC_Vector2D dir = gfc_vector2d(dx, dy);
    gfc_vector2d_normalize(&dir);
    gfc_vector2d_scale(self->velocity, dir, data->speed);
}

void monster_update(Entity *self)
{
    MonsterData *data;

    if(!self)return;
    self->frame += 0.05;
    if (self->frame >= 16) self->frame = 0;

    // move and collide with walls
    gfc_vector2d_add(self->position, self->position, self->velocity);
    entity_resolve_tile_collision(self, gCurrentLevel);

    data = (MonsterData*)self->data;
    if (data)
    {
        data->time_alive += 0.001;  // ~16ms per frame
        
        if (data->time_alive >= data->lifetime)
        {
            //slog("Monster despawning after %.2f seconds", data->time_alive);
            self->_delete_me = 1;
        }
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
