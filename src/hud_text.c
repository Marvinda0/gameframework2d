#include <SDL_ttf.h>
#include <stdio.h>

#include "simple_logger.h"
#include "gf2d_graphics.h"
#include "hud_text.h"

// TTF pipeline: TTF_Init -> TTF_OpenFont -> TTF_RenderText_Blended
// -> SDL_CreateTextureFromSurface -> SDL_SetTextureBlendMode -> SDL_RenderCopy
// docs: docs/hud.md

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

    // gfc_color8 stores 0-255 floats, cast directly — do NOT multiply by 255
    sdl_col.r = (Uint8)(color.r);
    sdl_col.g = (Uint8)(color.g);
    sdl_col.b = (Uint8)(color.b);
    sdl_col.a = (Uint8)(color.a);

    surf = TTF_RenderText_Blended(_font, text, sdl_col);
    if(!surf) return;

    // skip gf2d_graphics_screen_convert — it strips alpha and makes text invisible
    rend = gf2d_graphics_get_renderer();
    if(!rend) { SDL_FreeSurface(surf); return; }

    tex = SDL_CreateTextureFromSurface(rend, surf);
    SDL_FreeSurface(surf);
    if(!tex) return;

    // blend mode required or the text renders as a solid block
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
