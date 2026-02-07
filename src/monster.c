#include "simple_logger.h"

#include "monster.h"
#include "gfc_input.h"

void monster_think(Entity *self);
void monster_update(Entity *self);
void monster_free(Entity *self);

void monster_init_data(MonsterData *data, Entity *target)
{
    data->target = target;
}

Entity *monster_new(Entity *target)
{
    Entity *self;
    self = entity_new();
     if(!self)
    {
        slog("Error at Monster Entity Initialization");
        return NULL;
    }
    self->sprite = gf2d_sprite_load_all("images/space_bug_top.png", 128, 128, 17, 0);
    self->frame = 0;
    self->position = gfc_vector2d((gfc_random()*2 *600) - 600, (gfc_random()*2 *360) -360);
    self->rotation = 0;
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
    GFC_Vector2D difference;
    if(!self)return; 
    
    data = (MonsterData*)self->data;
    if(!data || !data->target) return;
    dx = data->target->position.x - self->position.x;
    dy = data->target->position.y - self->position.y;

    float angle = atan2(dy,dx);
    self->rotation = angle;
}

void monster_update(Entity *self)
{
    if(!self)return;
    self ->frame += 0.05;
    if (self->frame >= 16) self->frame = 0;

}

void monster_free(Entity *self)
{
    if(!self)return;
}
