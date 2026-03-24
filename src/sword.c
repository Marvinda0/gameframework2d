#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gfc_color.h"
#include "camera.h"
#include "sword.h"

#define SWORD_HALF_REACH  38.0f  // TODO(def): pixels forward/back from center (56px total depth)
#define SWORD_HALF_WIDTH  60.0f  // TODO(def): pixels to each side (120px total arc width)
#define SWORD_LIFETIME    0.15f  // TODO(def): seconds the visual stays visible

typedef struct
{
    float         lifetime;     // seconds remaining
    int           damage_dealt; // 0 = not yet hit, 1 = done
    GFC_Vector2D  direction;    // normalized forward
    int           damage;
} SwordData;

void sword_think(Entity *self, float dt);
void sword_update(Entity *self, float dt);
void sword_free(Entity *self);
void sword_draw_fn(Entity *self);

Entity *sword_swing_new(GFC_Vector2D position, GFC_Vector2D direction, int damage, Uint8 faction)
{
    Entity *self;
    SwordData *data;

    gfc_vector2d_normalize(&direction);

    self = entity_new();
    if(!self) return NULL;

    // place center of rect in front of attacker
    self->position = gfc_vector2d(
        position.x + direction.x * SWORD_HALF_REACH,
        position.y + direction.y * SWORD_HALF_REACH);

    self->faction      = faction;
    self->is_projectile = 0;
    self->damage       = damage;
    self->health       = 1;
    self->hit_radius   = 0.0f;  
    self->sprite       = NULL;
    self->draw         = sword_draw_fn;
    self->think        = sword_think;
    self->update       = sword_update;
    self->free         = sword_free;

    data = gfc_allocate_array(sizeof(SwordData), 1);
    if(!data) { slog("error allocating sword data"); return self; }
    data->lifetime     = SWORD_LIFETIME;
    data->damage_dealt = 0;
    data->direction    = direction;
    data->damage       = damage;
    self->data = data;

    return self;
}

void sword_think(Entity *self, float dt)
{
    SwordData *data;
    if(!self) return;
    data = (SwordData*)self->data;
    if(!data || data->damage_dealt) return;
    (void)dt;

    // deal damage once on the very first frame
    entity_damage_in_rect(
        self->position, data->direction,
        SWORD_HALF_REACH, SWORD_HALF_WIDTH,
        data->damage, self->faction, 0.5f,
        self->crit_chance, self->crit_dmg_mult, self->lifesteal);
    data->damage_dealt = 1;
}

void sword_update(Entity *self, float dt)
{
    SwordData *data;
    if(!self) return;
    data = (SwordData*)self->data;
    if(!data) return;
    data->lifetime -= dt;
    if(data->lifetime <= 0) self->_delete_me = 1;
}

void sword_draw_fn(Entity *self)
{
    SwordData *data;
    GFC_Vector2D offset, center, fwd, right;
    GFC_Vector2D c[4]; // corners
    GFC_Color col;

    if(!self) return;
    data = (SwordData*)self->data;
    if(!data) return;

    offset = camera_get_offset();
    center = gfc_vector2d(self->position.x + offset.x, self->position.y + offset.y);

    fwd   = data->direction;
    right = gfc_vector2d(-fwd.y, fwd.x);

    col = GFC_COLOR_YELLOW;

    // corners of the oriented rect
    c[0] = gfc_vector2d(center.x + fwd.x*SWORD_HALF_REACH + right.x*SWORD_HALF_WIDTH,
                        center.y + fwd.y*SWORD_HALF_REACH + right.y*SWORD_HALF_WIDTH);
    c[1] = gfc_vector2d(center.x + fwd.x*SWORD_HALF_REACH - right.x*SWORD_HALF_WIDTH,
                        center.y + fwd.y*SWORD_HALF_REACH - right.y*SWORD_HALF_WIDTH);
    c[2] = gfc_vector2d(center.x - fwd.x*SWORD_HALF_REACH - right.x*SWORD_HALF_WIDTH,
                        center.y - fwd.y*SWORD_HALF_REACH - right.y*SWORD_HALF_WIDTH);
    c[3] = gfc_vector2d(center.x - fwd.x*SWORD_HALF_REACH + right.x*SWORD_HALF_WIDTH,
                        center.y - fwd.y*SWORD_HALF_REACH + right.y*SWORD_HALF_WIDTH);

    gf2d_draw_line(c[0], c[1], col);
    gf2d_draw_line(c[1], c[2], col);
    gf2d_draw_line(c[2], c[3], col);
    gf2d_draw_line(c[3], c[0], col);
    // cross line through center to make the swing area more readable
    gf2d_draw_line(c[0], c[2], col);
}

void sword_free(Entity *self)
{
    if(!self) return;
    if(self->data)
    {
        free(self->data);
        self->data = NULL;
    }
}
/*eol@eof*/
