#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdlib.h>

typedef struct Projectile {
    int x;
    int y;
    int velocity;
} Projectile;

Projectile* new_projectile(int x, int y, int velocity);
void move_projectile(Projectile* projectile);

#endif
