#include <stdlib.h>
#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gfc_color.h"
#include "camera.h"
#include "entity.h"
#include "aoe_spell.h"

#define AOE_TICK_INTERVAL      0.5f
#define AOE_BURN_TICK_INTERVAL 0.15f

typedef struct
{
    float lifetime;      // seconds until the spell disappears
    float tick_timer;    // countdown to next damage tick
    float tick_interval; // seconds between ticks (varies by effect)
    float radius;        // aoe_radius from def — pixels
    int   damage;        // damage per tick (already calc'd by caller)
    int   effect;        // AoeEffect — drives tick behavior and ring color
} AoeData;

void aoe_think(Entity *self, float dt);
void aoe_update(Entity *self, float dt);
void aoe_free(Entity *self);
void aoe_draw_fn(Entity *self);

Entity *aoe_spell_new(GFC_Vector2D position, AbilityDef *def, Uint8 faction, int damage)
{
    Entity  *self;
    AoeData *data;

    if(!def) return NULL;

    self = entity_new();
    if(!self) return NULL;

    self->position      = position;
    self->faction       = faction;
    self->is_projectile = 0;
    self->hit_radius    = 0.0f;  // collision handled inside aoe_think, not the circle system
    self->sprite        = NULL;
    self->draw          = aoe_draw_fn;
    self->think         = aoe_think;
    self->update        = aoe_update;
    self->free          = aoe_free;

    data = gfc_allocate_array(sizeof(AoeData), 1);
    if(!data) { slog("aoe_spell: error allocating data"); return self; }

    data->lifetime      = def->lifetime;
    data->tick_timer    = 0;  // first tick fires immediately
    data->tick_interval = (def->aoe_effect == AOE_EFFECT_BURN)
                              ? AOE_BURN_TICK_INTERVAL
                              : AOE_TICK_INTERVAL;
    data->radius        = def->aoe_radius;
    data->damage        = damage;
    data->effect        = def->aoe_effect;
    self->data = data;

    return self;
}

void aoe_think(Entity *self, float dt)
{
    AoeData *data;
    if(!self) return;
    data = (AoeData*)self->data;
    if(!data) return;

    data->tick_timer -= dt;
    if(data->tick_timer <= 0)
    {
        switch(data->effect)
        {
            case AOE_EFFECT_HEAL:
                entity_heal_in_circle(self->position, data->radius,
                                      data->damage, self->faction);
                break;
            case AOE_EFFECT_BURN:
                entity_burn_in_circle(self->position, data->radius,
                                      data->damage, self->faction);
                break;
            case AOE_EFFECT_DAMAGE:
            case AOE_EFFECT_SLOW_ZONE:
            default:
                entity_damage_in_circle(self->position, data->radius,
                                        data->damage, self->faction, 0.3f);
                break;
        }
        data->tick_timer = data->tick_interval;
    }
}

void aoe_update(Entity *self, float dt)
{
    AoeData *data;
    if(!self) return;
    data = (AoeData*)self->data;
    if(!data) return;
    data->lifetime -= dt;
    if(data->lifetime <= 0) self->_delete_me = 1;
}

void aoe_free(Entity *self)
{
    if(!self || !self->data) return;
    free(self->data);
    self->data = NULL;
}

void aoe_draw_fn(Entity *self)
{
    AoeData     *data;
    GFC_Vector2D offset, screen_pos;
    float        pulse;  // 0..1 oscillating scale for inner ring

    if(!self) return;
    data = (AoeData*)self->data;
    if(!data) return;

    GFC_Color outer, inner;

    offset     = camera_get_offset();
    screen_pos = gfc_vector2d(self->position.x + offset.x,
                              self->position.y + offset.y);

    // choose ring colors based on the spell's effect type
    switch(data->effect)
    {
        case AOE_EFFECT_HEAL:
            outer = GFC_COLOR_GREEN;
            inner = GFC_COLOR_LIGHTGREEN;
            break;
        case AOE_EFFECT_BURN:
            outer = GFC_COLOR_RED;
            inner = GFC_COLOR_ORANGE;
            break;
        case AOE_EFFECT_SLOW_ZONE:
            outer = GFC_COLOR_MAGENTA;
            inner = gfc_color(1.0f, 0.4f, 1.0f, 1.0f);  // lavender
            break;
        case AOE_EFFECT_DAMAGE:
        default:
            outer = GFC_COLOR_CYAN;
            inner = GFC_COLOR_LIGHTBLUE;
            break;
    }

    // outer boundary ring
    gf2d_draw_circle(screen_pos, (int)data->radius, outer);

    // inner pulsing ring — shrinks as the tick timer counts down
    // clamp to at least 1: gf2d_draw_circle(radius=0) does malloc(0) then
    // immediately writes 32 bytes into it, corrupting the heap
    pulse = (data->tick_interval > 0)
                ? (data->tick_timer / data->tick_interval)
                : 0.5f;
    {
        int inner_r = (int)(data->radius * 0.5f * pulse);
        if(inner_r >= 1)
            gf2d_draw_circle(screen_pos, inner_r, inner);
    }
}

/*eof@eof*/
