#ifndef __HUD_H__
#define __HUD_H__

#include "entity.h"
#include "player.h"

/**
 * @brief Draw HP bar and (if open) the stats panel.
 *        Call every frame AFTER entity_system_draw().
 * @param player  Player entity (may be NULL).
 */
void hud_draw(Entity *player);

/**
 * @brief Toggle the stats panel open/closed.
 *        Bind this to a key press (e.g. I) in game.c.
 */
void hud_toggle_stats(void);

#endif
/*eol@eof*/
