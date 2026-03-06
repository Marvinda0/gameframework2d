#include "simple_logger.h"

#include "monster.h"
#include "gfc_input.h"

void monster_think(Entity *self);
void monster_update(Entity *self);
void monster_free(Entity *self);

void monster_init_data(MonsterData *data, Entity *target)
{
    data->target = target;
    data->lifetime = 3.0;
    data->time_alive = 0;
}

Entity *monster_new(Entity *target)
{
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
    self->sprite = gf2d_sprite_load_all("images/space_bug_top.png", 128, 128, 17, 0);
    self->frame = 0;
    gfc_vector2d_add(self->position, target->position, offset);    self->rotation = 0;
    self->velocity = gfc_vector2d(0,0);
    self->think = monster_think;
    self->update = monster_update;
    self->free = monster_free;
    self->data = gfc_allocate_array(sizeof(MonsterData),1);
    if(!self->data)slog("error");
    monster_init_data((MonsterData*)self->data, target);

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
}

void monster_update(Entity *self)
{
    MonsterData *data;

    if(!self)return;
    self ->frame += 0.05;
    if (self->frame >= 16) self->frame = 0;

    data = (MonsterData*)self->data;
    if (data)
    {
        data->time_alive += 0.02;  // ~16ms per frame
        
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
