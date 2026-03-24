#ifndef __HUD_TEXT_H__
#define __HUD_TEXT_H__

#include <SDL.h>
#include "gfc_color.h"

// thin SDL2_ttf wrapper — see docs/hud.md for pipeline notes

// load font and init TTF — call once at startup
void hud_text_init(const char *font_path, int pt_size);

// draw one line of text at screen pixel (x, y), top-left anchor
void hud_text_draw(const char *text, int x, int y, GFC_Color color);

// free font and quit TTF — call once at shutdown
void hud_text_free(void);

#endif
