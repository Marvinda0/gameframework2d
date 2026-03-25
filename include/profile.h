#ifndef __PROFILE_H__
#define __PROFILE_H__

#define PERM_SPEED      0   
#define PERM_LIFESTEAL  1   
#define PERM_HP         2   
#define PERM_XP_BONUS   3   
#define PERM_REVIVE     4   
#define PERM_COUNT      5

/* per-level delta applied to the live player entity */
#define PERM_SPEED_BONUS      0.25f   
#define PERM_LIFESTEAL_BONUS  0.02f   
#define PERM_HP_BONUS        100      
#define PERM_XP_MULT          0.25f   

/*
 * @brief Load profile.cfg into memory.
 *        Creates a fresh zero profile if the file does not exist.
 *        Call once at game startup before any profile queries.
 */
void profile_load(void);

/*
 * @brief Write the current profile to profile.cfg.
 *        Called automatically after every gold/upgrade change.
 *        Also call explicitly before quit for a final flush.
 */
void profile_save(void);

/* @brief Return the current gold balance. */
int  profile_get_gold(void);

/* @brief Add `amount` to the gold balance (use negative to subtract) then save. */
void profile_add_gold(int amount);

/* @brief Return the current level for upgrade idx (0 = never purchased). */
int  profile_get_upgrade(int idx);

/*
 * @brief Return the maximum allowed level for upgrade idx.
 *        Attempts to buy beyond this are silently ignored.
 */
int  profile_upgrade_max(int idx);

/* @brief Return the gold cost of the next level of upgrade idx. */
int  profile_upgrade_cost(int idx);

/*
 * @brief Attempt to purchase one level of upgrade idx.
 * @return 1 if purchased, 0 if already maxed or not enough gold.
 *         On success the profile is auto-saved.
 */
int  profile_buy_upgrade(int idx);

#endif
