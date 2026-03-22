#include "simple_logger.h"

#include <SDL.h>
#include <string.h>

#include "gf2d_graphics.h"

#include "simple_json.h"
#include "level.h"
#include "camera.h"

Level *gCurrentLevel = NULL;

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
    GFC_Vector2D offset;
    offset = camera_get_offset();
    if(!level)
    {
        slog("no world to draw");
        return;
    }
    gf2d_sprite_draw_image(level->background,gfc_vector2d(0,0));
    gf2d_sprite_draw_image(level->tileLayer , offset);
}

void level_setup_camera(Level *level)
{
    if(!level)return;
    if(!level->tileLayer || !level->tileLayer->surface)return;
    camera_set_bounds(gfc_rect(0,0,level->tileLayer->surface->w,level->tileLayer->surface->h));
    camera_apply_bounds();
    camera_enable_binding(1);
}

int level_get_tile_at(Level *level, float x, float y)
{
    int tx, ty, index;
    if(!level) return 0;

    //get tile column/row
    tx = (int)(x / level->tileSet->frame_w);
    ty = (int)(y / level->tileSet->frame_h);
    // out of bounds counts as solid wall
    if(tx < 0 || ty < 0 || tx >= level->width || ty >= level->height) return 1;
    // convert 2d tile coords to 1d array index
    index = tx + (ty * level->width);
    return level->tileMap[index];
}

Level *level_load(const char *filename, const char *level_name)
{
    SJson *json, *list, *item, *val;
    Level *level;
    const char *str;
    int i, j, count;
    int width = 75, height = 45, tile_w = 16, tile_h = 16, border_only = 0;
    char background[128] = {0};
    char tileset[128]    = {0};

    if(!filename || !level_name) return NULL;

    json = sj_load(filename);
    if(!json){ slog("level_load: could not open %s", filename); return NULL; }

    list = sj_object_get_value(json, "levels");
    if(!list){ sj_free(json); return NULL; }

    // find the level entry by name
    item = NULL;
    count = sj_array_get_count(list);
    for(i = 0; i < count; i++)
    {
        SJson *entry = sj_array_get_nth(list, i);
        const char *n = sj_object_get_value_as_string(entry, "name");
        if(n && strcmp(n, level_name) == 0){ item = entry; break; }
    }
    if(!item){ slog("level_load: level '%s' not found in %s", level_name, filename); sj_free(json); return NULL; }

    // read fields
    sj_object_get_value_as_int(item, "width",       &width);
    sj_object_get_value_as_int(item, "height",      &height);
    sj_object_get_value_as_int(item, "tile_w",      &tile_w);
    sj_object_get_value_as_int(item, "tile_h",      &tile_h);
    sj_object_get_value_as_int(item, "border_only", &border_only);

    str = sj_object_get_value_as_string(item, "background");
    if(str) strncpy(background, str, sizeof(background) - 1);

    str = sj_object_get_value_as_string(item, "tileset");
    if(str) strncpy(tileset, str, sizeof(tileset) - 1);

    level = level_new(width, height);
    if(!level){ sj_free(json); return NULL; }

    if(background[0]) level->background = gf2d_sprite_load_image(background);
    if(tileset[0])
        level->tileSet = gf2d_sprite_load_all(tileset, tile_w, tile_h, 1, 1);

    if(border_only)
    {
        // generate border walls, leave interior empty
        for(i = 0; i < width; i++)
        {
            level->tileMap[i]                      = 1; // top row
            level->tileMap[i + (height-1) * width] = 1; // bottom row
        }
        for(j = 0; j < height; j++)
        {
            level->tileMap[j * width]               = 1; // left col
            level->tileMap[j * width + (width - 1)] = 1; // right col
        }
    }
    else
    {
        // load tile array from JSON
        val = sj_object_get_value(item, "tiles");
        if(val)
        {
            int tile_count = sj_array_get_count(val);
            for(i = 0; i < tile_count && i < width * height; i++)
            {
                int t = 0;
                sj_get_integer_value(sj_array_get_nth(val, i), &t);
                level->tileMap[i] = (Uint8)t;
            }
        }
    }

    world_tile_layer_build(level);
    sj_free(json);
    slog("level_load: loaded '%s' (%dx%d)", level_name, width, height);
    return level;
}

