#include <stdio.h>
#include <string.h>

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

/* -----------------------------------------------------------------------
 * Action bar layout — 4 slots centred at bottom of screen (1200 × 720)
 * --------------------------------------------------------------------- */
#define BAR_SLOT_W      56
#define BAR_SLOT_H      56
#define BAR_SLOT_GAP     8
#define BAR_SLOT_COUNT   4
#define BAR_Y          650   /* top of slots; bottom edge at y=706            */
#define BAR_SCREEN_W  1200
#define BAR_CHAR_W       9   /* approx pixel width per 15-pt monospace char   */

static const char * const BAR_KEY_LABELS[BAR_SLOT_COUNT] = { "LMB", "RMB", "F", "R" };

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

static void hud_draw_ability_bar(Entity *player)
{
    PlayerData  *pdata;
    int          i, slot_x, total_w, start_x;
    float        pct, max_cd, cd;
    GFC_Rect     bg, overlay, border;
    char         buf[32];
    GFC_Color    col_ready  = gfc_color8(220, 220, 220, 255);  /* bright border when ready  */
    GFC_Color    col_oncd   = gfc_color8( 60,  60,  60, 200);  /* dim border while on CD    */
    GFC_Color    col_ovl    = gfc_color8(  0,   0,   0, 165);  /* dark sweep fill           */
    GFC_Color    col_cdsec  = gfc_color8(255, 185,  40, 255);  /* orange cooldown text      */
    GFC_Color    col_name   = gfc_color8(200, 200, 200, 190);  /* dim white ability name    */
    GFC_Color    col_key    = gfc_color8(140, 220, 255, 220);  /* light-blue key label      */

    if(!player || !player->data) return;
    pdata = (PlayerData*)player->data;

    total_w = BAR_SLOT_COUNT * BAR_SLOT_W + (BAR_SLOT_COUNT - 1) * BAR_SLOT_GAP;
    start_x = (BAR_SCREEN_W - total_w) / 2;

    for(i = 0; i < BAR_SLOT_COUNT; i++)
    {
        AbilityDef *adef = pdata->ability_defs[i];
        cd = (pdata->cooldowns[i] > 0.0f) ? pdata->cooldowns[i] : 0.0f;

        slot_x = start_x + i * (BAR_SLOT_W + BAR_SLOT_GAP);

        /* --- slot background --- */
        bg = gfc_rect(slot_x, BAR_Y, BAR_SLOT_W, BAR_SLOT_H);
        gf2d_draw_rect_filled(bg, gfc_color8(20, 20, 20, 210));

        if(adef)
        {
            /* --- cooldown sweep overlay: dark cap from top, shrinks as CD drains --- */
            if(adef->cooldown > 0.001f)
            {
                max_cd = adef->cooldown / 60.0f;
                if(pdata->attack_speed > 0.001f) max_cd /= pdata->attack_speed;
                pct = (cd > 0.0f) ? (cd / max_cd) : 0.0f;
                if(pct > 1.0f) pct = 1.0f;
                if(pct > 0.0f)
                {
                    int ov_h = (int)(BAR_SLOT_H * pct);
                    if(ov_h > 0)
                    {
                        overlay = gfc_rect(slot_x, BAR_Y, BAR_SLOT_W, ov_h);
                        gf2d_draw_rect_filled(overlay, col_ovl);
                    }
                }
            }

            /* --- ability short name — top-left, 6 chars max --- */
            snprintf(buf, sizeof(buf), "%.6s", adef->name);
            hud_text_draw(buf, slot_x + 3, BAR_Y + 3, col_name);

            /* --- CD seconds text — centred, only when ability is on cooldown --- */
            if(cd > 0.05f)
            {
                int tw;
                snprintf(buf, sizeof(buf), "%.1fs", cd);
                tw = (int)strlen(buf) * BAR_CHAR_W;
                hud_text_draw(buf,
                              slot_x + (BAR_SLOT_W - tw) / 2,
                              BAR_Y + BAR_SLOT_H / 2 - 7,
                              col_cdsec);
            }

            /* --- key binding — bottom-right inside slot --- */
            {
                int kw;
                snprintf(buf, sizeof(buf), "%s", BAR_KEY_LABELS[i]);
                kw = (int)strlen(buf) * BAR_CHAR_W;
                hud_text_draw(buf,
                              slot_x + BAR_SLOT_W - kw - 3,
                              BAR_Y + BAR_SLOT_H - 16,
                              col_key);
            }
        }
        else
        {
            /* empty slot — show key hint dimly so the player still sees the binding */
            int kw;
            snprintf(buf, sizeof(buf), "%s", BAR_KEY_LABELS[i]);
            kw = (int)strlen(buf) * BAR_CHAR_W;
            hud_text_draw(buf,
                          slot_x + BAR_SLOT_W - kw - 3,
                          BAR_Y + BAR_SLOT_H - 16,
                          gfc_color8(80, 80, 80, 150));
        }

        /* --- slot border: bright when ready, dim while on CD --- */
        border = gfc_rect(slot_x, BAR_Y, BAR_SLOT_W, BAR_SLOT_H);
        gf2d_draw_rect(border, (adef && cd > 0.0f) ? col_oncd : col_ready);
    }
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void hud_draw(Entity *player)
{
    if(!player) return;
    hud_draw_hp_bar(player);
    hud_draw_ability_bar(player);
    if(_stats_open)
        hud_draw_stats_panel(player);
}
/*eol@eof*/
