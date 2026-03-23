#ifndef __HUD_TEXT_H__
#define __HUD_TEXT_H__

#include <SDL.h>
#include "gfc_color.h"

/*
 * hud_text — thin SDL2_ttf wrapper for drawing strings on screen.
 *
 * Usage:
 *   hud_text_init("path/to/font.ttf", 16);  // once at startup
 *   hud_text_draw("HP: 100", 20, 20, GFC_COLOR_WHITE);
 *   hud_text_free();                         // once at shutdown
 *
 * Related docs: docs/hud.md
 */

/*
 * @brief Load the font and initialize SDL2_ttf.
 *        Safe to call even if TTF_Init was already called elsewhere.
 * @param font_path  Path to a .ttf file.
 * @param pt_size    Point size to load (e.g. 14 or 16).
 */
void hud_text_init(const char *font_path, int pt_size);

/*
 * @brief Draw a single line of text at screen coordinates (x, y).
 *        Top-left of the text will be at (x, y).
 * @param text   Null-terminated string.
 * @param x, y   Screen position in pixels.
 * @param color  GFC_Color (r,g,b,a each 0–1).
 */
void hud_text_draw(const char *text, int x, int y, GFC_Color color);

/*
 * @brief Free the loaded font and call TTF_Quit.
 */
void hud_text_free(void);

#endif
/*eol@eof*/
