#include "simple_logger.h"
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "gfc_input.h"
#include "gfc_color.h"
#include "gf2d_draw.h"

#include "player.h"
#include "defs.h"
#include "camera.h"
#include "level.h"
#include "projectile.h"
#include "aoe_spell.h"

#include "sword.h"
#include "shield_bash.h"
#include "hud.h"
#include "profile.h"

#define BARRAGE_COUNT    6      /* arrows fired per barrage burst */
#define BARRAGE_INTERVAL 0.08f  /* seconds between arrows in a burst (~12/s) */

// class rotation
static const char *_class_order[] = {"knight", "wizard", "ranger"};
static int         _class_index   = 0;

// melee dispatch table 
typedef Entity *(*MeleeFn)(GFC_Vector2D, GFC_Vector2D, int, Uint8);
typedef struct { const char *name; MeleeFn fn; } MeleeEntry;
static const MeleeEntry _melee_table[] = {
    {"sword_attack", sword_swing_new},
    {"shield_bash",  shield_bash_new},
    {NULL, NULL}
};

void player_think(Entity *self, float dt);
void player_update(Entity *self, float dt);
void player_free(Entity *self);
void player_draw(Entity *self);
void player_switch_class(Entity *self, const char *className);
static int  player_calc_dmg(PlayerData *pdata, int base, int *out_crit);
static int  player_calc_dmg_aoe(PlayerData *pdata, int base);
static Entity *player_fire_melee(Entity *self, AbilityDef *adef, GFC_Vector2D dir, int dmg);
static void player_fire_slot(Entity *self, PlayerData *pdata, int slot, GFC_Vector2D dir);
void player_apply_profile(Entity *self);

Entity *player_new()
{
    Entity *self;
    self = entity_new();
    if(!self)
    {
        slog("Error at Player Entity Initialization");
        return NULL;
    }
    self->sprite = gf2d_sprite_load_all("images/ed210.png", 128, 128, 16, 0);
    self->frame = 0;
    self->position = gfc_vector2d(600,360);

    self->think = player_think;
    self->update = player_update;
    self->free = player_free;
    self->draw = player_draw;
    self->faction = 0;
    self->hit_radius = 24.0f;
    self->damage = 0;

    PlayerData *pdata = gfc_allocate_array(sizeof(PlayerData), 1);
    if(!pdata) slog("error allocating player data");
    self->data = pdata;

    pdata->xp = 1000;
    _class_index = 0;
    player_switch_class(self, _class_order[_class_index]);
    /* revive is one-use-per-session — set here once so class switches can't reset it */
    pdata->has_revive = profile_get_upgrade(PERM_REVIVE);
    return self;
}

// non-AOE: applies crit roll, sets *out_crit (can be NULL)
static int player_calc_dmg(PlayerData *pdata, int base, int *out_crit)
{
    float dmg = (float)base * pdata->damage_mult;
    if(out_crit) *out_crit = 0;
    if(pdata->crit > 0.0f && ((float)rand() / (float)RAND_MAX) < pdata->crit)
    {
        dmg *= pdata->crit_dmg;
        if(out_crit) *out_crit = 1;
        slog("CRIT! base=%d final=%d crit_chance=%.2f", base, (int)dmg, pdata->crit);
    }
    return (dmg < 1.0f) ? 1 : (int)dmg;
}

// AOE: no crit — flat damage_mult only
static int player_calc_dmg_aoe(PlayerData *pdata, int base)
{
    int d = (int)((float)base * pdata->damage_mult);
    return d < 1 ? 1 : d;
}

void player_switch_class(Entity *self, const char *className)
{
    PlayerData *pdata;
    ClassDef *cdef;
    int i;
    if(!self || !self->data) return;
    pdata = (PlayerData*)self->data;
    cdef = defs_get_class(className);
    if(!cdef)
    {
        slog("player_switch_class: class '%s' not found", className);
        return;
    }
    slog("player_switch_class: '%s'", className);
    strncpy(pdata->className, cdef->name, sizeof(GFC_TextLine) - 1);
    self->health      = cdef->health;
    self->max_health  = cdef->health;
    self->armor       = cdef->armor;
    pdata->damage_mult  = cdef->damage_mult;
    pdata->ms           = 4.5f * cdef->ms;
    pdata->attack_speed = cdef->attack_speed;
    pdata->crit         = cdef->crit;
    pdata->crit_dmg     = cdef->crit_dmg;
    pdata->lifesteal    = cdef->lifesteal;
    pdata->knockback    = cdef->knockback;
    pdata->armor        = (float)cdef->armor;
    for(i = 0; i < 4; i++) pdata->ability_defs[i] = NULL;
    for(i = 0; i < cdef->ability_count && i < 4; i++)
        pdata->ability_defs[i] = defs_get_ability(cdef->abilities[i]);
    for(i = 0; i < 4; i++) pdata->cooldowns[i] = 0;
    pdata->charge_state = CHARGE_IDLE;
    pdata->charge_timer = 0;
    pdata->targeting_active = 0;
    pdata->targeting_slot   = 0;
    pdata->burst_remaining  = 0;
    pdata->burst_timer      = 0;
    self->color = GFC_COLOR_WHITE;
    /* apply permanent profile upgrades on top of fresh class base */
    player_apply_profile(self);
}

void player_apply_profile(Entity *self)
{
    PlayerData *pdata;
    if(!self || !self->data) return;
    pdata = (PlayerData*)self->data;
    /* add per-level bonuses on top of the class base stats */
    pdata->ms           += profile_get_upgrade(PERM_SPEED)     * PERM_SPEED_BONUS;
    pdata->lifesteal    += profile_get_upgrade(PERM_LIFESTEAL)  * PERM_LIFESTEAL_BONUS;
    pdata->pickup_range  = 30.0f; /* reserved, no pickups yet */
    self->max_health    += profile_get_upgrade(PERM_HP)         * PERM_HP_BONUS;
    self->health         = self->max_health;
    self->lifesteal      = pdata->lifesteal;
    /* NOTE: has_revive is intentionally NOT set here — it is one-use-per-session
       and must not be reset on class switch. Set it once in player_new instead. */
}

static Entity *player_fire_melee(Entity *self, AbilityDef *adef, GFC_Vector2D dir, int dmg)
{
    int i;
    for(i = 0; _melee_table[i].name; i++)
    {
        if(strcmp(adef->name, _melee_table[i].name) == 0)
            return _melee_table[i].fn(self->position, dir, dmg, self->faction);
    }
    slog("player: no melee handler for '%s'", adef->name);
    return NULL;
}

static void player_fire_slot(Entity *self, PlayerData *pdata, int slot, GFC_Vector2D dir)
{
    AbilityDef *adef = pdata->ability_defs[slot];
    int dmg;
    if(!adef) return;
    // compute base scaled damage — crit is rolled at HIT time by the entity system
    dmg = player_calc_dmg_aoe(pdata, adef->damage); // flat damage_mult, no crit roll
    switch(adef->type)
    {
        case ABILITY_TYPE_MELEE:
        {
            Entity *e = player_fire_melee(self, adef, dir, dmg);
            if(e) { e->crit_chance = pdata->crit; e->crit_dmg_mult = pdata->crit_dmg; }
            break;
        }
        case ABILITY_TYPE_PROJECTILE:
            if(strcmp(adef->name, "fire_blast") == 0)
            {
                float base_angle = atan2f(dir.y, dir.x);
                float offsets[3] = {-0.393f, 0.0f, 0.393f};
                int k;
                GFC_Vector2D d;
                for(k = 0; k < 3; k++)
                {
                    d = gfc_vector2d(cosf(base_angle + offsets[k]),
                                     sinf(base_angle + offsets[k]));
                    projectile_new_from_ability(self->position, d, adef, self->faction, dmg,
                                               pdata->crit, pdata->crit_dmg, pdata->lifesteal);
                }
            }
            else
                projectile_new_from_ability(self->position, dir, adef, self->faction, dmg,
                                           pdata->crit, pdata->crit_dmg, pdata->lifesteal);
            break;
        case ABILITY_TYPE_AOE:
            aoe_spell_new(self->position, adef, self->faction, dmg);
            break;
        case ABILITY_TYPE_BLINK:
        {
            GFC_Vector2D blink_dir = dir;
            gfc_vector2d_normalize(&blink_dir);
            self->position.x += blink_dir.x * adef->speed;
            self->position.y += blink_dir.y * adef->speed;
            self->invincible_timer = adef->lifetime;  // brief iframes
            break;
        }
        case ABILITY_TYPE_AOE_TARGETED:
            // handled by two-press logic in player_think — should not reach here
            break;
        default:
            break;
    }
    pdata->cooldowns[slot] = (adef->cooldown / 60.0f) / pdata->attack_speed;
}

void player_draw(Entity *self)
{
    PlayerData   *pdata;
    AbilityDef   *adef;
    GFC_Vector2D  offset, pos, center, screen_tgt;

    if(!self) return;

    // replicate the default entity_draw sprite rendering
    offset = camera_get_offset();
    pos    = gfc_vector2d(self->position.x + offset.x, self->position.y + offset.y);
    if(self->sprite)
    {
        center = gfc_vector2d(self->sprite->frame_w / 2.0f,
                              self->sprite->frame_h / 2.0f);
        gf2d_sprite_render(self->sprite, pos, &self->scale, &center,
                           &self->rotation, NULL, &self->color,
                           NULL, (Uint32)self->frame);
    }

    // draw blizzard targeting preview ring
    pdata = (PlayerData*)self->data;
    if(!pdata || !pdata->targeting_active) return;

    adef = pdata->ability_defs[pdata->targeting_slot];
    if(!adef) return;

    screen_tgt = gfc_vector2d(pdata->target_pos.x + offset.x,
                              pdata->target_pos.y + offset.y);
    gf2d_draw_circle(screen_tgt, (int)adef->aoe_radius, GFC_COLOR_CYAN);
    gf2d_draw_circle(screen_tgt, (int)(adef->aoe_radius * 0.5f), GFC_COLOR_LIGHTBLUE);
    gf2d_draw_circle(screen_tgt, 5, GFC_COLOR_WHITE);  // center dot
}

void player_give_xp(Entity *player, int amount)
{
    PlayerData *pdata;
    if(!player || !player->data) return;
    pdata = (PlayerData*)player->data;
    pdata->xp += amount;
}

void player_try_upgrade(Entity *player, int stat_index)
{
    PlayerData *pdata;
    if(!player || !player->data) return;
    pdata = (PlayerData*)player->data;
    if(pdata->xp < XP_COST_PER_UPGRADE) return;

    switch(stat_index)
    {
        case 0: // max HP +50
            if(player->max_health >= UPGRADE_CAP_MAX_HP) return;
            player->max_health += 50;
            player->health     += 50;
            if(player->health > player->max_health) player->health = player->max_health;
            break;
        case 1: // armor +1
            if(player->armor >= UPGRADE_CAP_ARMOR) return;
            player->armor++;
            pdata->armor = (float)player->armor;
            break;
        case 2: // damage mult +5%
            if(pdata->damage_mult >= UPGRADE_CAP_DMG) return;
            pdata->damage_mult += 0.1f;
            break;
        case 3: // crit chance +2%
            if(pdata->crit >= UPGRADE_CAP_CRIT) return;
            pdata->crit += 0.1f;
            break;
        case 4: // move speed +0.2
            if(pdata->ms >= UPGRADE_CAP_MS) return;
            pdata->ms += 0.5f;
            break;
        case 5: // attack speed +0.1
            if(pdata->attack_speed >= UPGRADE_CAP_ASPD) return;
            pdata->attack_speed += 0.5f;
            break;
        case 6: // lifesteal +2%
            if(pdata->lifesteal >= UPGRADE_CAP_LS) return;
            pdata->lifesteal += 0.02f;
            player->lifesteal = pdata->lifesteal;
            break;
        default: return;
    }
    pdata->xp -= XP_COST_PER_UPGRADE;
    fprintf(stderr, "[upgrade] stat=%d  new crit=%.2f  new dmg=%.2f  new aspd=%.2f  xp=%d\n",
            stat_index, pdata->crit, pdata->damage_mult, pdata->attack_speed, pdata->xp);
}

void player_think(Entity *self, float dt)
{
    int smx, smy;
    float mx = 0;
    float my = 0;
    GFC_Vector2D move, cam_pos, mouse_world, dir;
    PlayerData *pdata;

    if(!self) return;
    pdata = (PlayerData*)self->data;

    if (gfc_input_command_held("move_left"))  mx -= 0.1f;
    if (gfc_input_command_held("move_right")) mx += 0.1f;
    if (gfc_input_command_held("move_up"))    my -= 0.1f;
    if (gfc_input_command_held("move_down"))  my += 0.1f;

    move = gfc_vector2d(mx, my);
    gfc_vector2d_normalize(&move);
    gfc_vector2d_scale(self->velocity, move, pdata ? pdata->ms : 2.0f);

    // aim at mouse 
    SDL_GetMouseState(&smx, &smy);
    cam_pos = camera_get_position();
    mouse_world = gfc_vector2d(smx + cam_pos.x, smy + cam_pos.y);
    gfc_vector2d_sub(dir, mouse_world, self->position);
    // atan2 gives angle in radians, convert to degrees for the sprite renderer
    self->rotation = atan2f(dir.y, dir.x) * (180.0f / 3.14159f);

    // tick all cooldowns down by real elapsed seconds
    if(pdata)
    {
        if(pdata->cooldowns[0] > 0) pdata->cooldowns[0] -= dt;
        if(pdata->cooldowns[1] > 0) pdata->cooldowns[1] -= dt;
        if(pdata->cooldowns[2] > 0) pdata->cooldowns[2] -= dt;
        if(pdata->cooldowns[3] > 0) pdata->cooldowns[3] -= dt;

        // update targeting reticle to follow mouse every frame
        if(pdata->targeting_active)
            pdata->target_pos = mouse_world;

            if(gfc_input_command_pressed("switch_class"))
            {
                _class_index = (_class_index + 1) % 3;
                player_switch_class(self, _class_order[_class_index]);
            }

            // R = slot [3]: AOE_TARGETED uses two-press, others fire immediately
            if(gfc_input_command_pressed("ability_alt"))
            {
                AbilityDef *adef3 = pdata->ability_defs[3];
                if(adef3 && adef3->type == ABILITY_TYPE_AOE_TARGETED)
                {
                    if(!pdata->targeting_active && pdata->cooldowns[3] <= 0)
                    {
                        // first press: arm targeting preview
                        pdata->targeting_active = 1;
                        pdata->targeting_slot   = 3;
                        pdata->target_pos       = mouse_world;
                    }
                    else if(pdata->targeting_active)
                    {
                        // second press: cast at locked position
                        aoe_spell_new(pdata->target_pos, adef3,
                                      self->faction,
                                      player_calc_dmg_aoe(pdata, adef3->damage));
                        pdata->targeting_active = 0;
                        pdata->cooldowns[3] = (adef3->cooldown / 60.0f) / pdata->attack_speed;
                    }
                }
                else if(adef3 && pdata->cooldowns[3] <= 0)
                    player_fire_slot(self, pdata, 3, dir);
            }

            // left-click = slot [0]
            if((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) && pdata->cooldowns[0] <= 0)
                player_fire_slot(self, pdata, 0, dir);

            // right-click = slot [1]
            // barrage uses a burst mechanism; all other slot[1] abilities fire immediately
            {
                AbilityDef *adef1 = pdata->ability_defs[1];
                if(adef1 && strcmp(adef1->name, "barrage") == 0)
                {
                    if((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))
                       && pdata->cooldowns[1] <= 0 && pdata->burst_remaining == 0)
                    {
                        pdata->burst_remaining = BARRAGE_COUNT;
                        pdata->burst_timer     = 0.0f;  /* fire first arrow immediately */
                        pdata->burst_dir       = dir;   /* lock aim at trigger time */
                    }
                }
                else if((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) && pdata->cooldowns[1] <= 0)
                    player_fire_slot(self, pdata, 1, dir);
            }
            /* barrage burst ticker — runs every frame regardless of input */
            if(pdata->burst_remaining > 0)
            {
                AbilityDef *adef1 = pdata->ability_defs[1];
                pdata->burst_timer -= dt;
                if(pdata->burst_timer <= 0 && adef1)
                {
                    projectile_new_from_ability(self->position, pdata->burst_dir, adef1, self->faction, -1,
                                               pdata->crit, pdata->crit_dmg, pdata->lifesteal);
                    pdata->burst_remaining--;
                    if(pdata->burst_remaining > 0)
                        pdata->burst_timer = BARRAGE_INTERVAL;
                    else
                        pdata->cooldowns[1] = (adef1->cooldown / 60.0f) / pdata->attack_speed;
                }
            }

            // F = slot [2]
            if(gfc_input_command_held("charge") && pdata->charge_state == CHARGE_IDLE && pdata->cooldowns[2] <= 0)
            {
                AbilityDef *adef = pdata->ability_defs[2];
                if(adef && adef->type == ABILITY_TYPE_DASH)
                {
                    pdata->charge_dir = dir;
                    gfc_vector2d_normalize(&pdata->charge_dir);
                    if(strcmp(adef->name, "roll") == 0)
                    {
                        /* instant dash — no windup telegraph */
                        pdata->charge_state = CHARGE_DASHING;
                        pdata->charge_timer = adef->lifetime;
                    }
                    else
                    {
                        pdata->charge_state = CHARGE_WINDUP;
                        pdata->charge_timer = 0.333f;
                    }
                }
                else
                    player_fire_slot(self, pdata, 2, dir);
            }

            // DASH state machine — runs every frame, independent of input
            // only active when a DASH ability started it via charge_state
            if(pdata->charge_state == CHARGE_WINDUP)
            {
                self->velocity = gfc_vector2d(0.0f, 0.0f);
                self->color    = GFC_COLOR_YELLOW;
                pdata->charge_timer -= dt;
                if(pdata->charge_timer <= 0)
                {
                    pdata->charge_state = CHARGE_DASHING;
                    pdata->charge_timer = 0.25f;
                }
            }
            else if(pdata->charge_state == CHARGE_DASHING)
            {
                AbilityDef *adef = pdata->ability_defs[2];
                float dash_speed = adef ? adef->speed : 14.0f;
                gfc_vector2d_scale(self->velocity, pdata->charge_dir, dash_speed);
                self->invincible_timer = dt * 2.0f;
                self->color = GFC_COLOR_YELLOW;
                if(adef && adef->damage > 0)
                {
                    int dash_dmg = player_calc_dmg_aoe(pdata, adef->damage);
                    entity_damage_in_rect(
                        self->position, pdata->charge_dir,
                        28.0f, 20.0f,
                        dash_dmg,
                        self->faction, 0.333f,
                        pdata->crit, pdata->crit_dmg, pdata->lifesteal);
                }
                pdata->charge_timer -= dt;
                if(pdata->charge_timer <= 0)
                {
                    pdata->charge_state = CHARGE_IDLE;
                    pdata->cooldowns[2] = (adef ? adef->cooldown / 60.0f : 3.0f) / pdata->attack_speed;
                    self->color         = GFC_COLOR_WHITE;
                    self->invincible_timer = 0;
                }
            }
        }
    // keys 1-7 spend 5 XP each to upgrade a stat (one press = one upgrade)
    // blocked when the permanent upgrade shop overlay is open
    if(pdata && !hud_shop_is_open())
    {
        static Uint8 prev_keys[7] = {0};
        const Uint8 *ks = SDL_GetKeyboardState(NULL);
        static const SDL_Scancode codes[7] = {
            SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
            SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6,
            SDL_SCANCODE_7
        };
        int k;
        for(k = 0; k < 7; k++)
        {
            if(ks[codes[k]] && !prev_keys[k])
                player_try_upgrade(self, k);
            prev_keys[k] = ks[codes[k]];
        }
    }
}

void player_update(Entity *self, float dt)
{
    PlayerData *pdata;
    if(!self)return;
    pdata = (PlayerData*)self->data;
    self->frame += 6.0f * dt; // 6 fps at any framerate
    if (self->frame >= 16) self->frame = 0;
    self->position.x += self->velocity.x * dt * 60.0f;
    self->position.y += self->velocity.y * dt * 60.0f;
    entity_resolve_tile_collision(self, gCurrentLevel, dt);
    camera_center_on(self->position);
    /* revive — triggers when HP hits 0 and second-chance upgrade was purchased */
    if(self->health <= 0 && pdata && pdata->has_revive > 0)
    {
        self->health           = self->max_health / 2;
        self->invincible_timer = 3.0f; // 3 seconds of iframes so player can't immediately die again
        pdata->has_revive--;
        slog("REVIVE activated: HP restored to %d  revives_left=%d", self->health, pdata->has_revive);
    }
    else if(self->health <= 0)
    {
        self->_delete_me = 1; /* no revive left — truly dead */
    }
}
void player_free(Entity *self)
{
    if(!self)return;
    if(self->data)
    {
        free(self->data);
        self->data = NULL;
    }
}

/*eof@eof*/