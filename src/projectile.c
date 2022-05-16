#include "projectile.h"

Projectile* new_projectile(int x, int y, int velocity) {
    Projectile* to_return = malloc(sizeof(Projectile));

    to_return->x = x;
    to_return->y = y;
    to_return->velocity = velocity;

    return to_return;
}

void move_projectile(Projectile* projectile) {
    projectile->y -= projectile->velocity;
}
