#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"
#include "gfc_text.h"

typedef struct
{
    GFC_TextLine className;
    float health;
    float max_health;
    float damage;
    float ms;
    float armor;
    float attack_area_size;
    float attack_speed;
    float crit;
    float lifesteal;
    float knockback;
    void (*abilities[4])(Entity *self);
    float cooldowns[4];
    
} PlayerData;


/*
/ @brief Creates a new player entity
/ @return Player entity pointer or NULL on error
*/
Entity *player_new();

/*
/ @brief Return a vector with x and y of the palyer
/ @return Return a vector with x and y of the palyer
*/
GFC_Vector2D player_get_pos();

#endif