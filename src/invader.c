// TODO: this entire file needs a lot of refactoring
#include "invader.h"

#define INVADER_ROW_COUNT 5
#define INVADER_COLUMN_COUNT 15
#define MAX_INVADERS (INVADER_ROW_COUNT * INVADER_COLUMN_COUNT)

#define START_Y 20

// padding between each invader
#define X_PADDING 5
#define Y_PADDING 10

static LCDBitmap* invader_frames[2];
static Invader invaders[MAX_INVADERS];
static GameLostCallback game_lost;

void init_invader_data(GameLostCallback callback) {
    game_lost = callback;

    const char* err;
    invader_frames[0] = gfx->loadBitmap("Images/invader_1", &err);
    if (invader_frames[0] == NULL)
        sys->error("failed to load invader bitmap: %s", err);
    invader_frames[1] = gfx->loadBitmap("Images/invader_2", &err);
    if (invader_frames[1] == NULL)
        sys->error("failed to load invader bitmap: %s", err);

    int width, height;
    gfx->getBitmapData(invader_frames[0], &width, &height, NULL, NULL, NULL);
    const int x_offset = width + X_PADDING;
    const int y_offset = height + Y_PADDING;

    // so we can center the group horizontally
    const int group_width = (width * INVADER_COLUMN_COUNT) + (X_PADDING * (INVADER_COLUMN_COUNT - 1));
    const int start_x = ((SCREEN_WIDTH - group_width) / 2) + width / 2; // + width / 2 because sprites are drawn centered

    for (int i = 0; i < MAX_INVADERS; ++i) {
        int x = i % INVADER_COLUMN_COUNT;
        int y = i / INVADER_COLUMN_COUNT;
        invaders[i] = (Invader) {
            .sprite = spr->newSprite(),
            .position = (Vec2) {
                .x = start_x + x_offset * x,
                .y = START_Y + y_offset * y,
            },
            .alive = 1,
        };
        spr->moveTo(invaders[i].sprite, invaders[i].position.x, invaders[i].position.y);
        spr->setImage(invaders[i].sprite, invader_frames[0], kBitmapUnflipped);
        spr->setUserdata(invaders[i].sprite, &invaders[i]);
        spr->setTag(invaders[i].sprite, kTagEnemy);
        spr->setCollideRect(invaders[i].sprite, PDRectMake(0.f, 0.f, width, height));
        spr->addSprite(invaders[i].sprite);
    }
}

static int flipped = 0;
static int count = 0;
#define FRAMES_TO_FLIP 20
void update_invaders() {
    ++count;
    if (count >= FRAMES_TO_FLIP) {
        count = 0;
        flipped = ! flipped;

        for (int i = 0; i < MAX_INVADERS; ++i) {
            if (! invaders[i].alive) continue;
            spr->setImage(invaders[i].sprite, flipped ? invader_frames[1] : invader_frames[0], kBitmapUnflipped);
        }
    }
}

void free_invaders() {
    gfx->freeBitmap(invader_frames[0]);
    gfx->freeBitmap(invader_frames[1]);
    for (int i = 0; i < MAX_INVADERS; ++i) {
        spr->freeSprite(invaders[i].sprite);
    }
}
