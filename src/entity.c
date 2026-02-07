#include "simple_logger.h"

#include "entity.h"


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
    if(self->free)self->free(self->data);
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
}

void entity_draw(Entity *self)
{
    if(!self)
    {
        slog("cannot draw a NULL sprite");
        return;
    }
    if (self->sprite)
    {
        gf2d_sprite_render(
            self->sprite,
            self->position,
            NULL,
            NULL,
            &self->rotation,
            NULL,
            NULL,
            NULL,
            (Uint32)self->frame);
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
/*eof@eof*/