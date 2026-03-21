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

void entity_think(Entity *self)
{
    if(!self)return;
    if(self->think)self->think(self);
}

void entity_system_think()
{
    int i;
    for(i=0;i<_entity_manager.max_entities;i++)
    {
        if(!_entity_manager.entity_list[i]._inuse)continue;
        entity_think(&_entity_manager.entity_list[i]);
    }
}

void entity_update(Entity *self)
{
    if(!self)return;
    if(self->update)self->update(self);
}

void entity_system_update()
{
    int i;
    for(i=0;i<_entity_manager.max_entities;i++)
    {
        if(!_entity_manager.entity_list[i]._inuse)continue;
        entity_update(&_entity_manager.entity_list[i]);
    }

    for(i=0; i<_entity_manager.max_entities; i++)
    {
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
    if (self->sprite)
    {
        GFC_Vector2D center = gfc_vector2d(
            self->sprite->frame_w / 2.0,
            self->sprite->frame_h / 2.0
        );

        gf2d_sprite_render(
            self->sprite,
            pos,
            NULL,
            &center,
            &self->rotation,
            NULL,
            NULL,
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

void entity_resolve_tile_collision(Entity *self, Level *level)
{
    if(!self || !level) return;
    float r = 40; // close to half of 128px sprite size

    // only check the side we are moving toward to avoid double pushback
    if(self->velocity.x > 0 && level_get_tile_at(level, self->position.x + r, self->position.y))
        self->position.x -= self->velocity.x;
    else if(self->velocity.x < 0 && level_get_tile_at(level, self->position.x - r, self->position.y))
        self->position.x -= self->velocity.x;

    if(self->velocity.y > 0 && level_get_tile_at(level, self->position.x, self->position.y + r))
        self->position.y -= self->velocity.y;
    else if(self->velocity.y < 0 && level_get_tile_at(level, self->position.x, self->position.y - r))
        self->position.y -= self->velocity.y;
}
void entity_system_check_collisions()
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

            dist = gfc_vector2d_magnitude_between(a->position, b->position);
            if(dist > (a->hit_radius + b->hit_radius)) continue;

            // apply damage to a from b
            if(a->invincible_frames <= 0 && b->damage > 0)
            {
                a->health -= b->damage;
                a->invincible_frames = 30; // ~0.5s at 60fps
                if(a->health <= 0) a->_delete_me = 1;
            }
            // apply damage to b from a
            if(b->invincible_frames <= 0 && a->damage > 0)
            {
                b->health -= a->damage;
                b->invincible_frames = 30;
                if(b->health <= 0) b->_delete_me = 1;
            }

            // projectiles despawn on contact
            if(a->is_projectile) a->_delete_me = 1;
            if(b->is_projectile) b->_delete_me = 1;
        }
    }

    // tick down invincibility frames
    for(i = 0; i < (int)_entity_manager.max_entities; i++)
    {
        if(!_entity_manager.entity_list[i]._inuse) continue;
        if(_entity_manager.entity_list[i].invincible_frames > 0)
            _entity_manager.entity_list[i].invincible_frames--;
    }
}/*eof@eof*/