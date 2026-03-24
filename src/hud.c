#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "gf2d_draw.h"
#include "gfc_color.h"
#include "hud.h"
#include "hud_text.h"
#include "player.h"
#include "profile.h"

// hp bar position and size
#define HP_BAR_X        20
#define HP_BAR_Y       680
#define HP_BAR_W       200
#define HP_BAR_H        18
#define HP_BAR_PADDING   2

// stats panel layout
#define PANEL_X         20
#define PANEL_Y         20
#define PANEL_W        320
#define PANEL_LINE_H    22
#define PANEL_ROWS       9  // title + 7 stats + xp row
#define PANEL_PAD_X     10
#define PANEL_PAD_Y      8

// action bar layout (4 slots centred at bottom of a 1200x720 screen)
#define BAR_SLOT_W      56
#define BAR_SLOT_H      56
#define BAR_SLOT_GAP     8
#define BAR_SLOT_COUNT   4
#define BAR_Y          650
#define BAR_SCREEN_W  1200
#define BAR_CHAR_W       9  // approx px per char at 15pt monospace

static const char * const BAR_KEY_LABELS[BAR_SLOT_COUNT] = { "LMB", "RMB", "F", "R" };

static int _stats_open = 0;
static float _crit_flash = 0.0f;  // seconds remaining in crit flash
static int _shop_open = 0;

/* ---- shop layout constants ----------------------------------------------- */
#define SHOP_W       540
#define SHOP_H       340
#define SHOP_X       ((1200 - SHOP_W) / 2)   /* = 330 */
#define SHOP_Y       ((720  - SHOP_H) / 2)   /* = 190 */
#define SHOP_LINE_H   20
#define SHOP_PAD_X    14
#define SHOP_PAD_Y    12
#define SHOP_ENTRY_H  48   /* 2 text lines + spacing per upgrade row */
#define SHOP_COL_LV  320   /* x offset from SHOP_X for level column  */
#define SHOP_CHAR_W    9   /* approx px per character at 15pt mono    */

static const char * const SHOP_NAMES[PERM_COUNT] = {
    "Move Speed",
    "Lifesteal",
    "Max HP",
    "XP Bonus",
    "Second Chance"
};
static const char * const SHOP_DESCS[PERM_COUNT] = {
    "",
    "",
    "",
    "",
    ""
};

void hud_toggle_stats(void)
{
    _stats_open = !_stats_open;
}

void hud_toggle_shop(void)
{
    _shop_open = !_shop_open;
}

int hud_shop_is_open(void)
{
    return _shop_open;
}

void hud_trigger_crit_flash(void)
{
    _crit_flash = 0.25f;  // flash lasts 0.25 seconds
}

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
    GFC_Color    title_col = gfc_color8(255, 220,  80, 255);  // gold
    GFC_Color    value_col = gfc_color8(255, 255, 255, 255);  // white

    if(!player || !player->data) return;
    pdata = (PlayerData*)player->data;

    panel_h = PANEL_PAD_Y * 2 + PANEL_LINE_H * PANEL_ROWS;
    bg = gfc_rect(PANEL_X, PANEL_Y, PANEL_W, panel_h);
    gf2d_draw_rect_filled(bg, gfc_color8(10, 10, 10, 210));
    gf2d_draw_rect(bg, gfc_color8(180, 180, 180, 180));

    tx = PANEL_X + PANEL_PAD_X;
    ty = PANEL_Y + PANEL_PAD_Y;

    // title row
    snprintf(buf, sizeof(buf), "[ %s ]  STATS", pdata->className);
    hud_text_draw(buf, tx, ty, title_col);
    ty += PANEL_LINE_H + 4;

    // hp turns red below 30%
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
    ty += PANEL_LINE_H;

    snprintf(buf, sizeof(buf), "XP          %d  (5/upgrade)", pdata->xp);
    hud_text_draw(buf, tx, ty, gfc_color8(120, 255, 120, 255));  // green
}

static void hud_draw_ability_bar(Entity *player)
{
    PlayerData  *pdata;
    int          i, slot_x, total_w, start_x;
    float        pct, max_cd, cd;
    GFC_Rect     bg, overlay, border;
    char         buf[32];
    GFC_Color    col_ready  = gfc_color8(220, 220, 220, 255);  // bright border when ready
    GFC_Color    col_oncd   = gfc_color8( 60,  60,  60, 200);  // dim border while on CD
    GFC_Color    col_ovl    = gfc_color8(  0,   0,   0, 165);  // dark sweep fill
    GFC_Color    col_cdsec  = gfc_color8(255, 185,  40, 255);  // orange cooldown text
    GFC_Color    col_name   = gfc_color8(200, 200, 200, 190);  // dim white ability name
    GFC_Color    col_key    = gfc_color8(140, 220, 255, 220);  // light-blue key label

    if(!player || !player->data) return;
    pdata = (PlayerData*)player->data;

    total_w = BAR_SLOT_COUNT * BAR_SLOT_W + (BAR_SLOT_COUNT - 1) * BAR_SLOT_GAP;
    start_x = (BAR_SCREEN_W - total_w) / 2;

    for(i = 0; i < BAR_SLOT_COUNT; i++)
    {
        AbilityDef *adef = pdata->ability_defs[i];
        cd = (pdata->cooldowns[i] > 0.0f) ? pdata->cooldowns[i] : 0.0f;

        slot_x = start_x + i * (BAR_SLOT_W + BAR_SLOT_GAP);

        bg = gfc_rect(slot_x, BAR_Y, BAR_SLOT_W, BAR_SLOT_H);
        gf2d_draw_rect_filled(bg, gfc_color8(20, 20, 20, 210));

        if(adef)
        {
            // dark sweep cap that shrinks from top as CD drains
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

            // ability name top-left, 6 chars max
            snprintf(buf, sizeof(buf), "%.6s", adef->name);
            hud_text_draw(buf, slot_x + 3, BAR_Y + 3, col_name);

            // CD seconds, centred, only shown while on cooldown
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

            // key label bottom-right
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
            // empty slot — show key hint dimly
            int kw;
            snprintf(buf, sizeof(buf), "%s", BAR_KEY_LABELS[i]);
            kw = (int)strlen(buf) * BAR_CHAR_W;
            hud_text_draw(buf,
                          slot_x + BAR_SLOT_W - kw - 3,
                          BAR_Y + BAR_SLOT_H - 16,
                          gfc_color8(80, 80, 80, 150));
        }

        // bright border when ready, dim while on CD
        border = gfc_rect(slot_x, BAR_Y, BAR_SLOT_W, BAR_SLOT_H);
        gf2d_draw_rect(border, (adef && cd > 0.0f) ? col_oncd : col_ready);
    }
}

/* ---- upgrade shop overlay ----------------------------------------------- */

static void hud_draw_shop(Entity *player)
{
    GFC_Rect rect;
    char buf[80];
    int tx, ty, i, lv, maxlv, cost, gold, can_afford, maxed, name_len, cost_len;
    GFC_Color col_border = gfc_color8(200, 185,  50, 230);
    GFC_Color col_title  = gfc_color8(255, 215,   0, 255);  /* gold  */
    GFC_Color col_gold   = gfc_color8(255, 215,   0, 255);
    GFC_Color col_key    = gfc_color8(140, 220, 255, 255);  /* cyan  */
    GFC_Color col_name   = gfc_color8(220, 220, 220, 255);  /* white */
    GFC_Color col_maxed  = gfc_color8( 90,  90,  90, 255);  /* grey  */
    GFC_Color col_lv     = gfc_color8(100, 200, 200, 255);  /* teal  */
    GFC_Color col_aff    = gfc_color8(255, 185,  40, 255);  /* orange — can afford  */
    GFC_Color col_cant   = gfc_color8(200,  60,  60, 255);  /* red   — can't afford */
    GFC_Color col_desc   = gfc_color8(130, 130, 130, 255);
    GFC_Color col_sep    = gfc_color8( 90,  80,  20, 180);
    GFC_Color col_foot   = gfc_color8(160, 160, 160, 200);
    (void)player; /* player ptr reserved for future "sold-to" display */

    gold = profile_get_gold();

    /* --- background panel --- */
    rect = gfc_rect(SHOP_X, SHOP_Y, SHOP_W, SHOP_H);
    gf2d_draw_rect_filled(rect, gfc_color8(6, 6, 18, 245));
    gf2d_draw_rect(rect, col_border);
    rect = gfc_rect(SHOP_X + 2, SHOP_Y + 2, SHOP_W - 4, SHOP_H - 4);
    gf2d_draw_rect(rect, gfc_color8(60, 55, 10, 100));

    tx = SHOP_X + SHOP_PAD_X;
    ty = SHOP_Y + SHOP_PAD_Y;

    /* --- title row --- */
    {
        const char *title = "*  UPGRADE SHOP  *";
        int title_w = (int)strlen(title) * SHOP_CHAR_W;
        hud_text_draw(title, SHOP_X + (SHOP_W - title_w) / 2, ty, col_title);
    }
    ty += SHOP_LINE_H + 2;

    /* --- gold balance row --- */
    snprintf(buf, sizeof(buf), "Gold: %d", gold);
    hud_text_draw(buf, tx, ty, col_gold);
    ty += SHOP_LINE_H + 2;

    /* --- separator --- */
    rect = gfc_rect(tx, ty, SHOP_W - SHOP_PAD_X * 2, 1);
    gf2d_draw_rect_filled(rect, col_sep);
    ty += 6;

    /* --- 5 upgrade rows --- */
    for(i = 0; i < PERM_COUNT; i++)
    {
        lv       = profile_get_upgrade(i);
        maxlv    = profile_upgrade_max(i);
        cost     = profile_upgrade_cost(i);
        maxed    = (lv >= maxlv);
        can_afford = (!maxed && gold >= cost);

        /* line 1: [N] Name   Lv X/Y   Ng */
        snprintf(buf, sizeof(buf), "[%d]", i + 1);
        hud_text_draw(buf, tx, ty, col_key);

        hud_text_draw(SHOP_NAMES[i], tx + 32, ty, maxed ? col_maxed : col_name);

        if(maxed)
        {
            name_len = (int)strlen(SHOP_NAMES[i]) * SHOP_CHAR_W;
            hud_text_draw("MAXED", tx + 32 + name_len + 8, ty, col_maxed);
        }
        else
        {
            snprintf(buf, sizeof(buf), "Lv %d/%d", lv, maxlv);
            hud_text_draw(buf, SHOP_X + SHOP_COL_LV, ty, col_lv);

            snprintf(buf, sizeof(buf), "%dg", cost);
            cost_len = (int)strlen(buf) * SHOP_CHAR_W;
            hud_text_draw(buf,
                          SHOP_X + SHOP_W - SHOP_PAD_X - cost_len,
                          ty,
                          can_afford ? col_aff : col_cant);
        }
        ty += SHOP_LINE_H - 2;

        /* line 2: description */
        hud_text_draw(SHOP_DESCS[i], tx + 32, ty, col_desc);
        ty += SHOP_LINE_H + 8;
    }

    /* --- separator --- */
    rect = gfc_rect(tx, ty, SHOP_W - SHOP_PAD_X * 2, 1);
    gf2d_draw_rect_filled(rect, col_sep);
    ty += 6;

    /* --- footer --- */
    {
        const char *foot = "Press 1-5 to purchase   |   Press U to close";
        int foot_w = (int)strlen(foot) * SHOP_CHAR_W;
        hud_text_draw(foot, SHOP_X + (SHOP_W - foot_w) / 2, ty, col_foot);
    }
}

void hud_shop_update(Entity *player)
{
    static Uint8 prev_u  = 0;
    static Uint8 prev_k[5] = {0};
    const Uint8 *ks;
    static const SDL_Scancode codes[5] = {
        SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_4, SDL_SCANCODE_5
    };
    int i;
    PlayerData *pdata;

    ks = SDL_GetKeyboardState(NULL);

    /* U key — toggle shop open/close */
    if(ks[SDL_SCANCODE_U] && !prev_u)
        _shop_open = !_shop_open;
    prev_u = ks[SDL_SCANCODE_U];

    if(!_shop_open || !player || !player->data)
    {
        /* still update prev state so edge detection resets properly */
        for(i = 0; i < 5; i++) prev_k[i] = ks[codes[i]];
        return;
    }

    pdata = (PlayerData*)player->data;

    /* keys 1-5: buy permanent upgrades */
    for(i = 0; i < PERM_COUNT; i++)
    {
        if(ks[codes[i]] && !prev_k[i])
        {
            if(profile_buy_upgrade(i))
            {
                /* apply live delta to the running player */
                switch(i)
                {
                    case PERM_SPEED:
                        pdata->ms += PERM_SPEED_BONUS;
                        break;
                    case PERM_LIFESTEAL:
                        pdata->lifesteal += PERM_LIFESTEAL_BONUS;
                        player->lifesteal = pdata->lifesteal;
                        break;
                    case PERM_HP:
                        player->max_health += PERM_HP_BONUS;
                        player->health     += PERM_HP_BONUS;
                        if(player->health > player->max_health)
                            player->health = player->max_health;
                        break;
                    case PERM_XP_BONUS:
                        /* XP multiplier is applied in monster_free — no live delta needed */
                        break;
                    case PERM_REVIVE:
                        pdata->has_revive++;
                        break;
                    default: break;
                }
            }
        }
        prev_k[i] = ks[codes[i]];
    }
}

/* ---- main hud_draw ------------------------------------------------------- */

void hud_draw(Entity *player, float dt)
{
    if(!player) return;
    hud_draw_hp_bar(player);
    hud_draw_ability_bar(player);
    if(_stats_open)
        hud_draw_stats_panel(player);
    if(_shop_open)
        hud_draw_shop(player);

    // crit flash — fullscreen red overlay that fades over 0.25s
    if(_crit_flash > 0.0f)
    {
        Uint8 alpha = (Uint8)(160.0f * (_crit_flash / 0.25f));
        GFC_Rect screen = gfc_rect(0, 0, 1200, 720);
        gf2d_draw_rect_filled(screen, gfc_color8(220, 30, 30, alpha));
        _crit_flash -= dt;
        if(_crit_flash < 0.0f) _crit_flash = 0.0f;
    }
}
