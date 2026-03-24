#ifndef __HUD_H__
#define __HUD_H__

#include "entity.h"
#include "player.h"

// draw HP bar + action bar every frame; also draws stats panel / shop if open
void hud_draw(Entity *player, float dt);

// toggle the stats panel (I key)
void hud_toggle_stats(void);

// trigger a brief red screen flash — call when a crit lands
void hud_trigger_crit_flash(void);

// toggle the permanent upgrade shop overlay (U key)
void hud_toggle_shop(void);

// return 1 if the upgrade shop overlay is currently open
int  hud_shop_is_open(void);

/*
 * @brief Process shop input (U to toggle, 1-5 to buy upgrades) and apply
 *        live delta bonuses to the player on successful purchase.
 *        Call once per frame from game.c before entity_system_think.
 */
void hud_shop_update(Entity *player);

#endif
