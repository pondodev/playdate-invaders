#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdlib.h>

#include "game_math.h"

typedef struct Projectile {
    Vec2 position;
    int velocity;
    int frames_to_flip;
    int frames_since_flip;
    int flipped;
} Projectile;

Projectile* new_projectile(int x, int y, int velocity);
void move_projectile(Projectile* projectile);

#endif
