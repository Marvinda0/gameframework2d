#include "simple_logger.h"

#include "entity.h"
#include "camera.h"
#include "level.h"
#include "gf2d_draw.h"


typedef struct 
{
    Uint32 max_entities;
    Entity * entity_list;
}EntityManager;

static EntityManager _entity_manager = {0};

void entity_system_close();

void entity_system_init(Uint32 max)
{
    if(!max){
        slog("Cant initialize a system with zero or invalid number of entities");
        return;
    }
    _entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), max);
    if (!_entity_manager.entity_list)
    {
        slog("failed to alocate an entity list");
    }
    _entity_manager.max_entities = max;
    slog("Entity System Initialized");
    atexit(entity_system_close);
}

void entity_system_close()
{
    entity_clear_all(NULL);
    if (_entity_manager.entity_list)
    {
        free(_entity_manager.entity_list);
    }
    memset(&_entity_manager,0,sizeof(EntityManager));
    //_entity_manager.entity_list = NULL;
    //_entity_manager.max = 0;
    slog("Entity system closed");
}

/*
* @brief cleanup all active entities
*/
void entity_clear_all(Entity * ignore)
{
    int i;
    for(i=0;i<_entity_manager.max_entities;i++)
    {
        if(&_entity_manager.entity_list[i] == ignore)continue;
        if(!_entity_manager.entity_list[i]._inuse)continue;
        entity_free(&_entity_manager.entity_list[i]);
    }
}

/*
* @brief allocate an empty entity sruct for use
* @return NULL on error or pointer to blank entity
*/
Entity *entity_new()
{
    int i;
    for(i=0;i<_entity_manager.max_entities;i++)
    {
        if(_entity_manager.entity_list[i]._inuse)continue;
        memset(&_entity_manager.entity_list[i],0,sizeof(Entity));
        _entity_manager.entity_list[i]._inuse = 1;
        _entity_manager.entity_list[i]._delete_me = 0;  
        _entity_manager.entity_list[i].color = GFC_COLOR_WHITE;
        _entity_manager.entity_list[i].scale = gfc_vector2d(1,1);
        return &_entity_manager.entity_list[i];
    }
    slog("No available entities found");
    return NULL;
}

void entity_free(Entity *self)
{
    if(self == NULL)
    {
        slog("Cant Free Null entity");
        return;
    }
    gf2d_sprite_free(self->sprite);
    if(self->free)self->free(self);
    memset(self, 0, sizeof(Entity));
}

void entity_think(Entity *self, float dt)
{
    if(!self)return;
    if(self->think)self->think(self, dt);
}

void entity_system_think(float dt)
{
    int i;
    for(i=0;i<_entity_manager.max_entities;i++)
    {
        if(!_entity_manager.entity_list[i]._inuse)continue;
        entity_think(&_entity_manager.entity_list[i], dt);
    }
}

void entity_update(Entity *self, float dt)
{
    if(!self)return;
    if(self->update)self->update(self, dt);
}

void entity_system_update(float dt)
{
    int i;
    for(i=0;i<_entity_manager.max_entities;i++)
    {
        if(!_entity_manager.entity_list[i]._inuse)continue;
        entity_update(&_entity_manager.entity_list[i], dt);
    }

    for(i=0; i<_entity_manager.max_entities; i++)
    {
        if(!_entity_manager.entity_list[i]._inuse) continue;
        if(_entity_manager.entity_list[i]._delete_me)
        {
            entity_free(&_entity_manager.entity_list[i]);
        }
    }
}

void entity_draw(Entity *self)
{
    if(!self)
    {
        slog("cannot draw a NULL sprite");
        return;
    }
    GFC_Vector2D offset, pos;
    offset = camera_get_offset();
    gfc_vector2d_add(pos,self->position,offset);
    if(self->draw)
    {
        self->draw(self); // fully custom draw (handles its own camera offset)
    }
    else if (self->sprite)
    {
        GFC_Vector2D center = gfc_vector2d(
            self->sprite->frame_w / 2.0,
            self->sprite->frame_h / 2.0
        );

        gf2d_sprite_render(
            self->sprite,
            pos,
            &self->scale,
            &center,
            &self->rotation,
            NULL,
            &self->color,
            NULL,
            (Uint32)self->frame);
    }

    // debug hitbox
    if(self->hit_radius > 0)
    {
        gf2d_draw_circle(pos, (int)self->hit_radius, gfc_color(1,0,0,1));
    }
    
}

void entity_system_draw()
{
    int i;
    for(i=0;i<_entity_manager.max_entities;i++)
    {
        if(!_entity_manager.entity_list[i]._inuse)continue;
        entity_draw(&_entity_manager.entity_list[i]);
    }
}

void entity_resolve_tile_collision(Entity *self, Level *level, float dt)
{
    if(!self || !level) return;
    float r = self->hit_radius > 0 ? self->hit_radius : 14.0f;
    float dx = self->velocity.x * dt * 60.0f;
    float dy = self->velocity.y * dt * 60.0f;

    // only check the side we are moving toward to avoid double pushback
    if(self->velocity.x > 0 && level_get_tile_at(level, self->position.x + r, self->position.y))
        self->position.x -= dx;
    else if(self->velocity.x < 0 && level_get_tile_at(level, self->position.x - r, self->position.y))
        self->position.x -= dx;

    if(self->velocity.y > 0 && level_get_tile_at(level, self->position.x, self->position.y + r))
        self->position.y -= dy;
    else if(self->velocity.y < 0 && level_get_tile_at(level, self->position.x, self->position.y - r))
        self->position.y -= dy;
}

/* Apply armor reduction: damage * 100 / (100 + armor). Always at least 1. */
static int entity_apply_armor(int raw, int armor)
{
    int reduced = (armor > 0) ? (int)(raw * 100.0f / (100.0f + armor)) : raw;
    return (reduced < 1) ? 1 : reduced;
}

void entity_damage_in_rect(GFC_Vector2D center, GFC_Vector2D dir,
                           float half_reach, float half_width,
                           int damage, Uint8 attacking_faction, float iframes)
{
    int i;
    Entity *e;
    GFC_Vector2D local, perp;
    float along, side, r;

    perp = gfc_vector2d(-dir.y, dir.x); // perpendicular to forward

    for(i = 0; i < (int)_entity_manager.max_entities; i++)
    {
        e = &_entity_manager.entity_list[i];
        if(!e->_inuse || e->_delete_me) continue;
        if(e->faction == attacking_faction) continue; // don't hit friendlies
        if(e->is_projectile) continue;                // don't hit projectiles
        if(e->invincible_timer > 0) continue;         // already i-framed

        // project enemy center onto rect axes (OBB vs circle test)
        local = gfc_vector2d(e->position.x - center.x, e->position.y - center.y);
        along = local.x * dir.x  + local.y * dir.y;   // forward axis
        side  = local.x * perp.x + local.y * perp.y;  // side axis

        r = e->hit_radius; // expand rect bounds by enemy radius
        if(along >  (half_reach + r) || along < -(half_reach + r)) continue;
        if(side  >  (half_width + r) || side  < -(half_width + r)) continue;

        e->health -= entity_apply_armor(damage, e->armor);
        e->invincible_timer = iframes; // seconds
        if(e->health <= 0) e->_delete_me = 1;
    }
}
void entity_damage_in_circle(GFC_Vector2D center, float radius,
                             int damage, Uint8 attacking_faction, float iframes)
{
    int i;
    Entity *e;
    GFC_Vector2D delta;
    float dist_sq, max_dist;

    for(i = 0; i < (int)_entity_manager.max_entities; i++)
    {
        e = &_entity_manager.entity_list[i];
        if(!e->_inuse || e->_delete_me)   continue;
        if(e->faction == attacking_faction) continue;
        if(e->is_projectile)               continue;
        if(e->invincible_timer > 0)        continue;

        delta.x  = e->position.x - center.x;
        delta.y  = e->position.y - center.y;
        dist_sq  = delta.x * delta.x + delta.y * delta.y;
        max_dist = radius + e->hit_radius;
        if(dist_sq > max_dist * max_dist)  continue;

        e->health -= entity_apply_armor(damage, e->armor);
        e->invincible_timer = iframes;
        if(e->health <= 0) e->_delete_me = 1;
    }
}

void entity_heal_in_circle(GFC_Vector2D center, float radius,
                           int heal_amount, Uint8 faction)
{
    int i;
    Entity *e;
    GFC_Vector2D delta;
    float dist_sq, max_dist;

    for(i = 0; i < (int)_entity_manager.max_entities; i++)
    {
        e = &_entity_manager.entity_list[i];
        if(!e->_inuse || e->_delete_me) continue;
        if(e->faction != faction)        continue;  // only heal allies
        if(e->is_projectile)             continue;

        delta.x  = e->position.x - center.x;
        delta.y  = e->position.y - center.y;
        dist_sq  = delta.x * delta.x + delta.y * delta.y;
        max_dist = radius + e->hit_radius;
        if(dist_sq > max_dist * max_dist) continue;

        e->health += heal_amount;
        if(e->health > e->max_health) e->health = e->max_health;
    }
}

// burn bypasses armor and never sets iframes — each tick always lands
void entity_burn_in_circle(GFC_Vector2D center, float radius,
                           int damage, Uint8 attacking_faction)
{
    int i;
    Entity *e;
    GFC_Vector2D delta;
    float dist_sq, max_dist;

    for(i = 0; i < (int)_entity_manager.max_entities; i++)
    {
        e = &_entity_manager.entity_list[i];
        if(!e->_inuse || e->_delete_me)    continue;
        if(e->faction == attacking_faction) continue;
        if(e->is_projectile)               continue;

        // intentionally ignores invincible_timer 
        delta.x  = e->position.x - center.x;
        delta.y  = e->position.y - center.y;
        dist_sq  = delta.x * delta.x + delta.y * delta.y;
        max_dist = radius + e->hit_radius;
        if(dist_sq > max_dist * max_dist) continue;

        e->health -= damage;  // no armor reduction
        if(e->health <= 0) e->_delete_me = 1;
    }
}

void entity_knockback_in_rect(GFC_Vector2D center, GFC_Vector2D dir,
                              float half_reach, float half_width,
                              float force, Uint8 attacking_faction)
{
    int i;
    Entity *e;
    GFC_Vector2D local, perp;
    float along, side, r;

    perp = gfc_vector2d(-dir.y, dir.x);

    for(i = 0; i < (int)_entity_manager.max_entities; i++)
    {
        e = &_entity_manager.entity_list[i];
        if(!e->_inuse || e->_delete_me) continue;
        if(e->faction == attacking_faction) continue;
        if(e->is_projectile) continue;

        local = gfc_vector2d(e->position.x - center.x, e->position.y - center.y);
        along = local.x * dir.x  + local.y * dir.y;
        side  = local.x * perp.x + local.y * perp.y;

        r = e->hit_radius;
        if(along >  (half_reach + r) || along < -(half_reach + r)) continue;
        if(side  >  (half_width + r) || side  < -(half_width + r)) continue;

        // push enemy directly along the bash direction
        e->position.x += dir.x * force;
        e->position.y += dir.y * force;
    }
}

void entity_system_check_collisions(float dt)
{
    int i, j;
    Entity *a, *b;
    float dist;

    // check all pairs
    for(i = 0; i < (int)_entity_manager.max_entities - 1; i++)
    {
        a = &_entity_manager.entity_list[i];
        if(!a->_inuse || a->_delete_me) continue;

        for(j = i + 1; j < (int)_entity_manager.max_entities; j++)
        {
            b = &_entity_manager.entity_list[j];
            if(!b->_inuse || b->_delete_me) continue;

            // only interact across factions
            if(a->faction == b->faction) continue;

            // projectiles pass through each other
            if(a->is_projectile && b->is_projectile) continue;

            dist = gfc_vector2d_magnitude_between(a->position, b->position);
            if(dist > (a->hit_radius + b->hit_radius)) continue;

            // apply damage to a from b
            if(a->invincible_timer <= 0 && b->damage > 0)
            {
                a->health -= entity_apply_armor(b->damage, a->armor);
                a->invincible_timer = 0.833f; // ~50 frames at 60fps
                if(a->health <= 0) a->_delete_me = 1;
            }
            // apply damage to b from a
            if(b->invincible_timer <= 0 && a->damage > 0)
            {
                b->health -= entity_apply_armor(a->damage, b->armor);
                b->invincible_timer = 0.5f;   // ~30 frames at 60fps
                if(b->health <= 0) b->_delete_me = 1;
            }

            // projectiles despawn on contact
            if(a->is_projectile) a->_delete_me = 1;
            if(b->is_projectile) b->_delete_me = 1;
        }
    }

    // tick down invincibility timer
    for(i = 0; i < (int)_entity_manager.max_entities; i++)
    {
        if(!_entity_manager.entity_list[i]._inuse) continue;
        if(_entity_manager.entity_list[i].invincible_timer > 0)
            _entity_manager.entity_list[i].invincible_timer -= dt;
    }
}/*eof@eof*/