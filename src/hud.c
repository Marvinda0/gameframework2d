#include <stdio.h>

#include "gf2d_draw.h"
#include "gfc_color.h"
#include "hud.h"
#include "hud_text.h"
#include "player.h"

/* -----------------------------------------------------------------------
 * HP bar layout — anchored bottom-left, all values in screen pixels
 * --------------------------------------------------------------------- */
#define HP_BAR_X        20
#define HP_BAR_Y       680
#define HP_BAR_W       200
#define HP_BAR_H        18
#define HP_BAR_PADDING   2

/* -----------------------------------------------------------------------
 * Stats panel layout
 * --------------------------------------------------------------------- */
#define PANEL_X         20
#define PANEL_Y         20
#define PANEL_W        320
#define PANEL_LINE_H    22   /* pixels per text row          */
#define PANEL_ROWS       8   /* title + 7 stat rows          */
#define PANEL_PAD_X     10   /* text indent from panel edge  */
#define PANEL_PAD_Y      8   /* gap above first row          */

static int _stats_open = 0;  /* 0 = closed, 1 = open */

void hud_toggle_stats(void)
{
    _stats_open = !_stats_open;
}

/* -----------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------- */

static void hud_draw_hp_bar(Entity *player)
{
    float pct;
    int fill_w;
    GFC_Rect bg, fill, outline;

    pct = (player->max_health > 0)
        ? (float)player->health / (float)player->max_health
        : 0.0f;
    if(pct < 0.0f) pct = 0.0f;
    if(pct > 1.0f) pct = 1.0f;

    fill_w = (int)((HP_BAR_W - HP_BAR_PADDING * 2) * pct);

    bg = gfc_rect(HP_BAR_X, HP_BAR_Y, HP_BAR_W, HP_BAR_H);
    gf2d_draw_rect_filled(bg, gfc_color8(30, 30, 30, 220));

    if(fill_w > 0)
    {
        fill = gfc_rect(
            HP_BAR_X + HP_BAR_PADDING,
            HP_BAR_Y + HP_BAR_PADDING,
            fill_w,
            HP_BAR_H - HP_BAR_PADDING * 2);
        gf2d_draw_rect_filled(fill, gfc_color8(200, 40, 40, 255));
    }

    outline = gfc_rect(HP_BAR_X, HP_BAR_Y, HP_BAR_W, HP_BAR_H);
    gf2d_draw_rect(outline, gfc_color8(255, 255, 255, 200));
}

static void hud_draw_stats_panel(Entity *player)
{
    PlayerData  *pdata;
    GFC_Rect     bg;
    char         buf[64];
    int          panel_h;
    int          tx, ty;
    GFC_Color    title_col = gfc_color8(255, 220,  80, 255);  /* gold  */
    GFC_Color    value_col = gfc_color8(255, 255, 255, 255);  /* white */

    if(!player || !player->data) return;
    pdata = (PlayerData*)player->data;

    /* --- background panel --- */
    panel_h = PANEL_PAD_Y * 2 + PANEL_LINE_H * PANEL_ROWS;
    bg = gfc_rect(PANEL_X, PANEL_Y, PANEL_W, panel_h);
    gf2d_draw_rect_filled(bg, gfc_color8(10, 10, 10, 210));
    gf2d_draw_rect(bg, gfc_color8(180, 180, 180, 180));

    tx = PANEL_X + PANEL_PAD_X;
    ty = PANEL_Y + PANEL_PAD_Y;

    /* title */
    snprintf(buf, sizeof(buf), "[ %s ]  STATS", pdata->className);
    hud_text_draw(buf, tx, ty, title_col);
    ty += PANEL_LINE_H + 4;

    /* HP — turns red below 30% */
    snprintf(buf, sizeof(buf), "HP          %d / %d", player->health, player->max_health);
    hud_text_draw(buf, tx, ty,
        (player->health < player->max_health * 0.3f) ? gfc_color8(255,80,80,255) : value_col);
    ty += PANEL_LINE_H;

    snprintf(buf, sizeof(buf), "Armor       %d", player->armor);
    hud_text_draw(buf, tx, ty, value_col);
    ty += PANEL_LINE_H;

    snprintf(buf, sizeof(buf), "Damage      %.0f%%", pdata->damage_mult * 100.0f);
    hud_text_draw(buf, tx, ty, value_col);
    ty += PANEL_LINE_H;

    snprintf(buf, sizeof(buf), "Crit        %.0f%%  (x%.1f)", pdata->crit * 100.0f, pdata->crit_dmg);
    hud_text_draw(buf, tx, ty, value_col);
    ty += PANEL_LINE_H;

    snprintf(buf, sizeof(buf), "Move Speed  %.1f", pdata->ms);
    hud_text_draw(buf, tx, ty, value_col);
    ty += PANEL_LINE_H;

    snprintf(buf, sizeof(buf), "Atk Speed   %.1fx", pdata->attack_speed);
    hud_text_draw(buf, tx, ty, value_col);
    ty += PANEL_LINE_H;

    snprintf(buf, sizeof(buf), "Lifesteal   %.0f%%", pdata->lifesteal * 100.0f);
    hud_text_draw(buf, tx, ty, value_col);
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void hud_draw(Entity *player)
{
    if(!player) return;
    hud_draw_hp_bar(player);
    if(_stats_open)
        hud_draw_stats_panel(player);
}
/*eol@eof*/
