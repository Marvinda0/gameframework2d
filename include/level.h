#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <SDL.h>

#include "gf2d_sprite.h"

typedef struct
{
    Sprite  *background;        // the background
    Sprite  *tileLayer;         // prerendered tile layer
    Sprite  *tileSet;           // Sprite containing the tile set to draw with
    Uint8   *tileMap;           // Pointer to the tile map data
    Uint32  width;
    Uint32  height;        

}Level ;

Level *level_test_new();

/*
* @brief Create the level given the parameters for each level
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


/*
@brief Allocate memory for a new level
@return NULL on error or Pointer to the level in memory
*/
Level *level_new();

/*
*given a level, get the index of the tileMap for the tile´s coordinate
* @return 
*/
int level_get_tile_index(Level *level, Uint32 x, Uint32 y);

/*
@brief Deallocate memory for a previously created level
@param level the level we want to free
*/
void level_free(Level *level);

/*
@brief Draw the level
@param level to draw 
*/
void level_draw(Level *level);

/*
@brief setup level camera
@param level to setup camera 
*/
void level_setup_camera(Level *level);


#endif