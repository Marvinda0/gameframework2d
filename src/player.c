#include "simple_logger.h"

#include "player.h"
#include "projectile.h"
#include "defs.h"
#include "gfc_input.h"
#include "camera.h"
#include "level.h"

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

    // combat stats
    self->health = 100;
    self->max_health = 100;
    self->damage = 0;  // player deals no contact damage, only via projectiles
    self->faction = 0;
    self->hit_radius = 48.0f;

    // player data
    PlayerData *pdata = gfc_allocate_array(sizeof(PlayerData), 1);
    if(!pdata) slog("error allocating player data");
    pdata->damage = 20.0f;
    pdata->ms = 2.0f;
    pdata->attack_speed = 20.0f; // cooldown in frames
    self->data = pdata;

    return self;
}

void player_think(Entity *self)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    int smx, smy;
    float mx = 0;
    float my = 0;
    GFC_Vector2D move, cam_pos, mouse_world, dir;
    PlayerData *pdata;

    if(!self) return;
    pdata = (PlayerData*)self->data;

    // movement
    if (keys[SDL_SCANCODE_LEFT])  mx -= 0.1;
    if (keys[SDL_SCANCODE_RIGHT]) mx += 0.1;
    if (keys[SDL_SCANCODE_UP])    my -= 0.1;
    if (keys[SDL_SCANCODE_DOWN])  my += 0.1;

    move = gfc_vector2d(mx, my);
    gfc_vector2d_normalize(&move);
    gfc_vector2d_scale(self->velocity, move, pdata ? pdata->ms : 2.0f);

    // aim at mouse (convert screen space to world space)
    // SDL mouse coords are screen pixels, add camera position to get world coords
    // then subtract player world pos to get a direction vector toward the cursor
    SDL_GetMouseState(&smx, &smy);
    cam_pos = camera_get_position();
    mouse_world = gfc_vector2d(smx + cam_pos.x, smy + cam_pos.y);
    gfc_vector2d_sub(dir, mouse_world, self->position);
    // atan2 gives angle in radians, convert to degrees for the sprite renderer
    self->rotation = atan2f(dir.y, dir.x) * (180.0f / 3.14159f);

    // cooldown[0] = basic attack, ticks down each frame
    if(pdata)
    {
        if(pdata->cooldowns[0] > 0) pdata->cooldowns[0] -= 1.0f;

        if((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) && pdata->cooldowns[0] <= 0)
        {
            // look up ability def so damage/speed/sprite come from the JSON file
            AbilityDef *adef = defs_get_ability("basic_shot");
            projectile_new_from_ability(self->position, dir, adef, self->faction);
            pdata->cooldowns[0] = adef ? adef->cooldown : 20.0f;
        }
    }
}

void player_update(Entity *self)
{
    if(!self)return;
    self ->frame += 0.1;
    if (self->frame >= 16) self->frame = 0;
    gfc_vector2d_add(self->position,self->position,self->velocity);
    entity_resolve_tile_collision(self, gCurrentLevel);
    camera_center_on(self->position);

}
void player_free(Entity *self)
{
    if(!self)return;
    if(self->data)
    {
        free(self->data);
        self->data = NULL;
    }
}

/*eof@eof*/