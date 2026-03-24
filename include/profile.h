#ifndef __PROFILE_H__
#define __PROFILE_H__

/*
 * profile.h — persistent player profile (gold + permanent upgrades)
 *
 * Saved to profile.cfg in the working directory as JSON.
 * Loaded once at startup, auto-saved whenever gold or upgrades change.
 */

/* permanent upgrade slot indices */
#define PERM_SPEED      0   /* Move Speed    – max 5 tiers, 100g/tier  +0.25 ms       */
#define PERM_LIFESTEAL  1   /* Lifesteal     – max 3 tiers, 100g/tier  +2% lifesteal  */
#define PERM_HP         2   /* Max HP        – max 5 tiers, 150g/tier  +100 HP        */
#define PERM_XP_BONUS   3   /* XP Bonus      – max 3 tiers, 200g/tier  +25% XP gain  */
#define PERM_REVIVE     4   /* Second Chance – max 3, 500g each,  +1 revive per run  */
#define PERM_COUNT      5

/* per-level delta applied to the live player entity */
#define PERM_SPEED_BONUS      0.25f   /* ms added per speed level              */
#define PERM_LIFESTEAL_BONUS  0.02f   /* lifesteal fraction added per level    */
#define PERM_HP_BONUS        100      /* max HP added per level                */
#define PERM_XP_MULT          0.25f   /* XP multiplier fraction added per lv  */

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
 * @return 1 if purchased, 0 if already maxed or insufficient gold.
 *         On success the profile is auto-saved.
 */
int  profile_buy_upgrade(int idx);

#endif
