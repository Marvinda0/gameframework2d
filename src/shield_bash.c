#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gfc_color.h"
#include "camera.h"
#include "shield_bash.h"

#define BASH_HALF_SIZE    50.0f  // square hitbox — 100x100px total
#define BASH_LIFETIME     0.133f // TODO(def): seconds visible (~8 frames at 60fps)
#define BASH_KNOCKBACK    60.0f  // pixels enemies are pushed along bash direction

typedef struct
{
    float        lifetime;      // seconds remaining
    int          damage_dealt;  // 0 = not yet hit, 1 = done
    GFC_Vector2D direction;     // normalized forward
    int          damage;        // TODO(def): scale with armor stat when implemented
} BashData;

void bash_think(Entity *self, float dt);
void bash_update(Entity *self, float dt);
void bash_free(Entity *self);
void bash_draw_fn(Entity *self);

Entity *shield_bash_new(GFC_Vector2D position, GFC_Vector2D direction, int damage, Uint8 faction)
{
    Entity *self;
    BashData *data;

    gfc_vector2d_normalize(&direction);

    self = entity_new();
    if(!self) return NULL;

    // place center of square in front of attacker
    self->position = gfc_vector2d(
        position.x + direction.x * BASH_HALF_SIZE,
        position.y + direction.y * BASH_HALF_SIZE);

    self->faction       = faction;
    self->is_projectile = 0;
    self->damage        = damage;
    self->health        = 1;
    self->hit_radius    = 0.0f;   // OBB handled manually
    self->sprite        = NULL;
    self->draw          = bash_draw_fn;
    self->think         = bash_think;
    self->update        = bash_update;
    self->free          = bash_free;

    data = gfc_allocate_array(sizeof(BashData), 1);
    if(!data) { slog("error allocating bash data"); return self; }
    data->lifetime     = BASH_LIFETIME;
    data->damage_dealt = 0;
    data->direction    = direction;
    data->damage       = damage;
    self->data = data;

    return self;
}

void bash_think(Entity *self, float dt)
{
    BashData *data;
    if(!self) return;
    data = (BashData*)self->data;
    if(!data || data->damage_dealt) return;
    (void)dt;

    // deal damage and knock back on first frame only
    entity_damage_in_rect(
        self->position, data->direction,
        BASH_HALF_SIZE, BASH_HALF_SIZE,
        data->damage, self->faction, 0.333f,
        self->crit_chance, self->crit_dmg_mult, self->lifesteal);

    entity_knockback_in_rect(
        self->position, data->direction,
        BASH_HALF_SIZE, BASH_HALF_SIZE,
        BASH_KNOCKBACK, self->faction);

    data->damage_dealt = 1;
}

void bash_update(Entity *self, float dt)
{
    BashData *data;
    if(!self) return;
    data = (BashData*)self->data;
    if(!data) return;
    data->lifetime -= dt;
    if(data->lifetime <= 0) self->_delete_me = 1;
}

void bash_draw_fn(Entity *self)
{
    BashData *data;
    GFC_Vector2D offset, center, fwd, right;
    GFC_Vector2D c[4];
    GFC_Color col;

    if(!self) return;
    data = (BashData*)self->data;
    if(!data) return;

    offset = camera_get_offset();
    center = gfc_vector2d(self->position.x + offset.x, self->position.y + offset.y);

    fwd   = data->direction;
    right = gfc_vector2d(-fwd.y, fwd.x);

    // fade from cyan to dark as lifetime ticks down
    col = GFC_COLOR_CYAN;

    // corners of the square OBB
    c[0] = gfc_vector2d(center.x + fwd.x*BASH_HALF_SIZE + right.x*BASH_HALF_SIZE,
                        center.y + fwd.y*BASH_HALF_SIZE + right.y*BASH_HALF_SIZE);
    c[1] = gfc_vector2d(center.x + fwd.x*BASH_HALF_SIZE - right.x*BASH_HALF_SIZE,
                        center.y + fwd.y*BASH_HALF_SIZE - right.y*BASH_HALF_SIZE);
    c[2] = gfc_vector2d(center.x - fwd.x*BASH_HALF_SIZE - right.x*BASH_HALF_SIZE,
                        center.y - fwd.y*BASH_HALF_SIZE - right.y*BASH_HALF_SIZE);
    c[3] = gfc_vector2d(center.x - fwd.x*BASH_HALF_SIZE + right.x*BASH_HALF_SIZE,
                        center.y - fwd.y*BASH_HALF_SIZE + right.y*BASH_HALF_SIZE);

    gf2d_draw_line(c[0], c[1], col);
    gf2d_draw_line(c[1], c[2], col);
    gf2d_draw_line(c[2], c[3], col);
    gf2d_draw_line(c[3], c[0], col);
    // arrow line showing push direction
    gf2d_draw_line(center,
                   gfc_vector2d(center.x + fwd.x*BASH_HALF_SIZE,
                                center.y + fwd.y*BASH_HALF_SIZE),
                   col);
}

void bash_free(Entity *self)
{
    if(!self) return;
    if(self->data)
    {
        free(self->data);
        self->data = NULL;
    }
}
/*eol@eof*/
