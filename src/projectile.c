#include "projectile.h"

Projectile* new_projectile(int x, int y, int velocity) {
    Projectile* to_return = malloc(sizeof(Projectile));

    to_return->position = (Vec2) {
        .x = x,
        .y = y
    };
    to_return->velocity = velocity;
    to_return->frames_to_flip = 5;
    to_return->frames_since_flip = 0;
    to_return->flipped = 0;

    return to_return;
}

void move_projectile(Projectile* projectile) {
    projectile->position.y -= projectile->velocity;
}
