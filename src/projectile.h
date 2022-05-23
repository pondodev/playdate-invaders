#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdlib.h>

#include "pd_api.h"

#include "globals.h"
#include "game_math.h"
#include "invader.h"

typedef struct Projectile {
    LCDSprite* sprite;
    Vec2 position;
    int velocity;
    int frames_since_flip;
    int flipped;
} Projectile;

void init_projectiles(void);
Projectile* new_projectile(int x, int y, int velocity);
int update_projectile(Projectile* projectile);
void free_projectile(Projectile* projectile);

#endif
