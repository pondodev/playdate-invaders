#include "projectile.h"

#define FRAMES_TO_FLIP 5

static LCDBitmap* image = NULL;
static int image_width = 0;
static int image_height = 0;

static SpriteCollisionResponseType on_projectile_collide(LCDSprite* projectile, LCDSprite* other);

void init_projectiles() {
    const char* err;
    image = gfx->loadBitmap("Images/projectile", &err);
    if (image == NULL) {
        sys->error("failed to load projectile bitmap: %s", err);
    }

    gfx->getBitmapData(image, &image_width, &image_height, NULL, NULL, NULL);
}

Projectile* new_projectile(int x, int y, int velocity) {
    Projectile* to_return = malloc(sizeof(Projectile));

    to_return->sprite = spr->newSprite();
    spr->setImage(to_return->sprite, image, kBitmapUnflipped);
    spr->setTag(to_return->sprite, kTagProjectile);
    spr->setCollideRect(to_return->sprite, PDRectMake(0.f, 0.f, image_width, image_height));
    spr->setCollisionsEnabled(to_return->sprite, 1);
    spr->setCollisionResponseFunction(to_return->sprite, on_projectile_collide);
    spr->setUserdata(to_return->sprite, to_return);
    spr->moveTo(to_return->sprite, x, y);
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

int update_projectile(Projectile* projectile) {
    projectile->position.y -= projectile->velocity;
    int len;
    SpriteCollisionInfo* collisions = spr->moveWithCollisions(projectile->sprite, projectile->position.x, projectile->position.y, NULL, NULL, &len);

    int collided = 0;
    for (int i = 0; i < len; ++i) {
        SpriteCollisionInfo c = collisions[i];
        CollisionTag tag = spr->getTag(c.other);
        if (tag == kTagEnemy) {
            Invader* enemy = (Invader*)spr->getUserdata(c.other);
            enemy->alive = 0;
            spr->removeSprite(c.other);
            collided = 1;
        }
    }

    sys->realloc(collisions, 0); // free collisions array

    if (++projectile->frames_since_flip > FRAMES_TO_FLIP) {
        projectile->frames_since_flip = 0;
        projectile->flipped = ! projectile->flipped;
        spr->setImageFlip(projectile->sprite, projectile->flipped ? kBitmapFlippedX : kBitmapUnflipped);
    }

    return collided;
}

static SpriteCollisionResponseType on_projectile_collide(LCDSprite* projectile, LCDSprite* other) {
    return kCollisionTypeOverlap;
}

void free_projectile(Projectile* projectile) {
    spr->freeSprite(projectile->sprite);
    free(projectile);
}
