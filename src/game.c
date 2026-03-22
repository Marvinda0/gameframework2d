#include <SDL.h>
#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "projectile.h"
#include "defs.h"
#include "level.h"
#include "camera.h"

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    Entity *Player;
    Level *level;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(255,100,255,200);
    
    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN manumps ====---");
    gfc_input_init("/Users/jesusgarcia/Documents/git/gameframework2d/gfc/sample_config/input.cfg");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    SDL_ShowCursor(SDL_DISABLE);
    camera_set_size(gfc_vector2d(1200,720));

    /*Entitities*/
    entity_system_init(1024);
    /*demo setup*/
    defs_load_all(); // load enemies.def and abilities.def
    Player = player_new();

    //LEVEL
    level = level_load("defs/levels.def", "dungeon_1");
    if(!level) level = level_test_new(); // fallback
    gCurrentLevel = level;
    level_setup_camera(level);
    
    
    //TEST ENEMIES
    int i;
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    for(i=0;i<2;i++)
    {
        monster_new(Player, "basic_melee");
    }
    slog("press [escape] to quit");
    /*main game loop*/
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first            
            // Entities
            level_draw(level);
            entity_system_think();
            entity_system_update();
            entity_system_check_collisions();
            entity_system_draw();

            
            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    level_free(level);
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
