#include "simple_logger.h"

#include "player.h"
#include "gfc_input.h"
#include "camera.h"

void player_think(Entity *self);
void player_update(Entity *self);
void player_free(Entity *self);

Entity *player_new()
{
    Entity *self;
    self = entity_new();
    if(!self)
    {
        slog("Error at Player Entity Initialization");
        return NULL;
    }
    self->sprite = gf2d_sprite_load_all("images/ed210.png", 128, 128, 16, 0);
    self->frame = 0;
    self->position = gfc_vector2d(600,360);

    self->think = player_think;
    self->update = player_update;
    self->free = player_free;

    return self;
}

void player_think(Entity *self)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    float mx=0; 
    float my=0;
    GFC_Vector2D move;
    if(!self)return; 
    if (keys[SDL_SCANCODE_LEFT])
    {
        mx -= 0.1;
    }
    if (keys[SDL_SCANCODE_RIGHT])
    {
        mx += 0.1;
    }
    if (keys[SDL_SCANCODE_UP])
    {
        my -= 0.1;
    }
    if (keys[SDL_SCANCODE_DOWN])
    {
        my += 0.1;
    }
    move = gfc_vector2d(mx,my);
    gfc_vector2d_normalize(&move);
    gfc_vector2d_scale(self->velocity,move,2);
}

void player_update(Entity *self)
{
    if(!self)return;
    self ->frame += 0.1;
    if (self->frame >= 16) self->frame = 0;
    gfc_vector2d_add(self->position,self->position,self->velocity);
    camera_center_on(self->position);

}
void player_free(Entity *self)
{
    if(!self)return;
}

/*eof@eof*/