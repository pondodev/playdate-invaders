#include "projectile.h"

#define FRAMES_TO_FLIP 5

static LCDBitmap* image = NULL;

void init_projectiles() {
    const char* err;
    image = gfx->loadBitmap("Images/projectile", &err);
    if (image == NULL) {
        sys->error("failed to load projectile bitmap: %s", err);
    }
}

Projectile* new_projectile(int x, int y, int velocity) {
    Projectile* to_return = malloc(sizeof(Projectile));

    to_return->sprite = spr->newSprite();
    spr->setImage(to_return->sprite, image, kBitmapUnflipped);
    spr->setTag(to_return->sprite, kTagProjectile);
    spr->setUserdata(to_return->sprite, to_return);
    spr->addSprite(to_return->sprite);

    to_return->position = (Vec2) {
        .x = x,
        .y = y
    };
    to_return->velocity = velocity;
    to_return->frames_since_flip = 0;
    to_return->flipped = 0;

    return to_return;
}

void update_projectile(Projectile* projectile) {
    projectile->position.y -= projectile->velocity;
    spr->moveTo(projectile->sprite, projectile->position.x, projectile->position.y);
    if (++projectile->frames_since_flip > FRAMES_TO_FLIP) {
        projectile->frames_since_flip = 0;
        projectile->flipped = ! projectile->flipped;
        spr->setImageFlip(projectile->sprite, projectile->flipped ? kBitmapFlippedX : kBitmapUnflipped);
    }
}
