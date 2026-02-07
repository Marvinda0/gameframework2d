#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <SDL.h>

#include "gf2d_sprite.h"

typedef struct
{
    Sprite  *background;        // the background
    Sprite  *tileSet;           //Sprite containing the tile set to draw with
    Uint8   *tileMap;           // Pointer to the tile map data
    Uint32  width,height;       // size of tile map

}Level ;

/*
* @brief background 
* @param tileSet
* @param tileWidth
* @param tileHeight
* @param tilesPerLine
* @param width
* @param height
* @param
*
*/
Level *level_create(
    const char *background, 
    const char *tileSet, 
    Uint32 tileWidth, 
    Uint32 tileHeight,  
    Uint32 tilesPerLine,
    Uint32 width, 
    Uint32 height);

Level *level_new();

/*
*given a level, get the index of the tileMap for the tile´s coordinate
* @return 
*/
int level_get_tile_index(Level *level, Uint32 x, Uint32 y)

void level_free(Level *level);

void level_draw(Level *level);

#endif