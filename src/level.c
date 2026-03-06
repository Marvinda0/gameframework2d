#include "simple_logger.h"

#include <SDL.h>

#include "level.h"
#include "gf2d_graphics.h"

void world_tile_layer_build(Level *level)
{
    int i,j;
    Uint32 frame;
    Uint32 index;
    GFC_Vector2D position;

    if(!level)return;

    if(!level->tileSet)
    {
        slog("No tile layer can be created wthout a tile Set");
        return;
    }

    if(level->tileLayer)
    {
        gf2d_sprite_free(level->tileLayer);
    }
    level->tileLayer = gf2d_sprite_new();
    
    level->tileLayer->surface = gf2d_graphics_create_surface(level->width * level->tileSet->frame_w,level->height * level->tileSet->frame_h);

    level->tileLayer->frame_w = level->width * level->tileSet->frame_w;
    level->tileLayer->frame_h = level->height * level->tileSet->frame_h;
    
    if(!level->tileLayer->surface)
    {
        slog("Surface for tileyaer was not created"); 
        return;
    }

    for(j=0;j<level->height;j++)
    {
        for(i=0;i<level->width;i++)
        {
            index = i + (j * level->width);
            if(level->tileMap[index] == 0)continue;
            position.x = i*level->tileSet->frame_w;
            position.y = j*level->tileSet->frame_h;
            frame = level->tileMap[index] - 1; 
    
    
            gf2d_sprite_draw_to_surface(
                level->tileSet,
                position,
                NULL,
                NULL,
                frame,
                level->tileLayer->surface);
            }
    }
    level->tileLayer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(),level->tileLayer->surface);
    if (!level->tileSet->texture)
    {
        slog("failed to convert level tilelayer surface to textur");
        return; 
    }
}

Level *level_test_new()
{
    int i,j;
    int width =75, height =45;
    Level *level;

    level = level_new(width,height);

    if(!level)return NULL;

    level->background = gf2d_sprite_load_image("images/backgrounds/castle.png");
    level->tileSet = gf2d_sprite_load_all(
        "images/backgrounds/tileset1.png",
        16,
        16,
        1,
        1
    );
    for(i=0;i<width;i++)
    {
        level->tileMap[i] = 1;
        level->tileMap[(i+(height-1)*width)] = 1;
    }
    for(i=0;i<height;i++)
    {
        level->tileMap[(i*width)] = 1;
        level->tileMap[(i*width+(width-1))] = 1;
    }
    world_tile_layer_build(level);
    return level;
}
 


Level *level_new(Uint32 width, Uint32 height)
{
    Level *level;

    if(!width || !height){
        slog("No width or height received");
        return NULL;
    }

    level = gfc_allocate_array(sizeof(Level),1);
    if(!level)
    {
        slog("Failed to create Level");
        return NULL;
    }
    //boilerplate 
    level->tileMap = gfc_allocate_array(sizeof(Uint8),width*height);
    level->height = height;
    level->width = width;
    
    return level;
}

Level *level_create(
    const char *background, 
    const char *tileSet, 
    Uint32 tileWidth, 
    Uint32 tileHeight,  
    Uint32 tilesPerLine,
    Uint32 width, 
    Uint32 height)
{
    char *b;
    Level *level;
    if((!width)||(!height))
    {
        slog("Error explain");
        return NULL;
    }
    //level = level_new();
    if(!level)return NULL;
    if(background)
    {
        level->background = gf2d_sprite_load_image(background);
    }
    
}

int level_get_tile_index(Level *level, Uint32 x, Uint32 y)
{
    
}

void level_add_border(Level *level, Uint8 tile)
{

}

void level_free(Level *level)
{
    if (!level) return;
    gf2d_sprite_free(level->background);
    gf2d_sprite_free(level->tileSet);
    gf2d_sprite_free(level->tileLayer);
    if(level->tileMap)free(level->tileMap);
    free(level);
}

void level_draw(Level *level)
{
    if(!level)
    {
        slog("no world to draw");
        return;
    }
    gf2d_sprite_draw_image(level->background,gfc_vector2d(0,0));
    gf2d_sprite_draw_image(level->tileLayer ,gfc_vector2d(0,0));
}

