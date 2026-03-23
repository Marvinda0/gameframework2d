#include <SDL_ttf.h>
#include <stdio.h>

#include "simple_logger.h"
#include "gf2d_graphics.h"
#include "hud_text.h"

/*
 * TTF rendering pipeline (per Professor Kehoe's notes):
 *
 *   1. TTF_Init()
 *   2. TTF_OpenFont()  → TTF_Font *
 *   3. TTF_RenderText_Blended()  → SDL_Surface *  (ARGB, anti-aliased)
 *   4. gf2d_graphics_screen_convert()  → converts surface to renderer pixel format
 *   5. SDL_CreateTextureFromSurface()  → SDL_Texture *
 *   6. SDL_RenderCopy()  → draws it
 *   7. SDL_DestroyTexture() + SDL_FreeSurface()  → free per-frame resources
 *
 * References:
 *   https://wiki.libsdl.org/SDL2_ttf/TTF_RenderText_Blended
 *   https://wiki.libsdl.org/SDL2_ttf/TTF_Init
 *   https://www.deusinmachina.net/p/sdl-tutorial-part-2-drawing-text
 */

static TTF_Font *_font = NULL;

void hud_text_init(const char *font_path, int pt_size)
{
    if(TTF_Init() == -1)
    {
        slog("hud_text_init: TTF_Init failed: %s", TTF_GetError());
        return;
    }
    _font = TTF_OpenFont(font_path, pt_size);
    if(!_font)
        slog("hud_text_init: could not open font '%s': %s", font_path, TTF_GetError());
    else
        slog("hud_text_init: loaded '%s' at %dpt", font_path, pt_size);
}

void hud_text_draw(const char *text, int x, int y, GFC_Color color)
{
    SDL_Surface  *surf   = NULL;
    SDL_Texture  *tex    = NULL;
    SDL_Renderer *rend   = NULL;
    SDL_Rect      dst;
    SDL_Color     sdl_col;

    if(!_font || !text || text[0] == '\0') return;

    /* gfc_color8 stores channels as raw 0-255 floats (not 0.0-1.0 normalized) */
    sdl_col.r = (Uint8)(color.r);
    sdl_col.g = (Uint8)(color.g);
    sdl_col.b = (Uint8)(color.b);
    sdl_col.a = (Uint8)(color.a);

    /* TTF_RenderText_Blended → ARGB8888 surface with proper alpha */
    surf = TTF_RenderText_Blended(_font, text, sdl_col);
    if(!surf) return;

    /* Upload directly to GPU — skip gf2d_graphics_screen_convert because
     * the screen surface format typically has no alpha channel, which would
     * strip the transparency and make the text invisible.                   */
    rend = gf2d_graphics_get_renderer();
    if(!rend) { SDL_FreeSurface(surf); return; }

    tex = SDL_CreateTextureFromSurface(rend, surf);
    SDL_FreeSurface(surf);
    if(!tex) return;

    /* Must enable alpha blending on the texture or text renders as a solid block */
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    dst.x = x;
    dst.y = y;
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(rend, tex, NULL, &dst);

    SDL_DestroyTexture(tex);
}

void hud_text_free(void)
{
    if(_font)
    {
        TTF_CloseFont(_font);
        _font = NULL;
    }
    TTF_Quit();
}
/*eol@eof*/
