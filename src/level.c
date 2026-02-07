#include "simple_logger.h"

#include <SDL.h>

#include "gf2d_sprite.h"

typedef struct
{
    Sprite  *background;        // the background
    Sprite  *tileSet;           //Sprite containing the tile set to draw with
    Uint8   *tileMap;           // Pointer to the tile map data
    Uint32  width,height;       // size of tile map

}Level ;

Level *level_new()
{
    Level *level;
    level = gfc_allocate_array(sizeof(Level),1);
    if(!level) return NULL;
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
    level = level_new();
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
    if(level->tileMap)free(level->tileMap);
    free(level);
}

//void level_draw(Level *level)
