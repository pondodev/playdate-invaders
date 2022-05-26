// TODO: this entire file needs a lot of refactoring
#include "invader.h"

#define INVADER_ROW_COUNT 5
#define INVADER_COLUMN_COUNT 15
#define MAX_INVADERS (INVADER_ROW_COUNT * INVADER_COLUMN_COUNT)

#define START_Y 20

// padding between each invader
#define X_PADDING 5
#define Y_PADDING 10

// movement/animation related
static int flipped = 0;
static int frame_count = 0;
#define MIN_SPEED 0.1f
#define MAX_SPEED 5.f
static float speed = MIN_SPEED;
static int moving_right = 1;
static int should_move_down = 0;
#define MIN_FRAMES_TO_FLIP 5
#define MAX_FRAMES_TO_FLIP 20
static int frames_to_flip = MAX_FRAMES_TO_FLIP;
#define MIN_SPACE_FROM_EDGE 20
#define FINISH_LINE SCREEN_HEIGHT - 40
static int invaders_killed = 0;

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

    for (int i = 0; i < MAX_INVADERS; ++i) {
        invaders[i] = (Invader) {
            .sprite = spr->newSprite(),
            .alive = 1,
        };
        spr->setImage(invaders[i].sprite, invader_frames[0], kBitmapUnflipped);
        spr->setUserdata(invaders[i].sprite, &invaders[i]);
        spr->setTag(invaders[i].sprite, kTagEnemy);
        spr->setCollideRect(invaders[i].sprite, PDRectMake(0.f, 0.f, width, height));
        spr->addSprite(invaders[i].sprite);
    }
}

void reset_invaders() {
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
        invaders[i].position = (Vec2) {
            .x = start_x + x_offset * x,
            .y = START_Y + y_offset * y,
        };
        spr->moveTo(invaders[i].sprite, invaders[i].position.x, invaders[i].position.y);

        if (! invaders[i].alive) {
            invaders[i].alive = 1;
            spr->addSprite(invaders[i].sprite);
        }
    }

    speed = MIN_SPEED;
    frames_to_flip = MAX_FRAMES_TO_FLIP;
    invaders_killed = 0;
}

void update_invaders() {
    ++frame_count;
    int image_changed = 0;
    if (frame_count >= frames_to_flip) {
        image_changed = 1;
        frame_count = 0;
        flipped = ! flipped;
    }

    const Vec2 velocity = {
        .x = moving_right ? speed : -speed,
        .y = should_move_down ? 10.f : 0.f,
    };
    should_move_down = 0;

    for (int i = 0; i < MAX_INVADERS; ++i) {
        if (! invaders[i].alive) continue;

        invaders[i].position.x += velocity.x;
        invaders[i].position.y += velocity.y;
        if (invaders[i].position.y >= FINISH_LINE) {
            game_lost();
            break; // don't need to keep checking now that the lose condition has been met
        }
        spr->moveTo(invaders[i].sprite, invaders[i].position.x, invaders[i].position.y);

        int dist;
        if (moving_right) dist = SCREEN_WIDTH - invaders[i].position.x;
        else dist = invaders[i].position.x;

        if (dist <= MIN_SPACE_FROM_EDGE) {
            moving_right = ! moving_right;
            should_move_down = 1;
        }

        if (image_changed)
            spr->setImage(invaders[i].sprite, flipped ? invader_frames[1] : invader_frames[0], kBitmapUnflipped);
    }
}

void on_invader_kill() {
    ++invaders_killed;
    // speed adjustment
    {
        const int min = 0;
        const int max = MAX_INVADERS - (MAX_INVADERS / INVADER_ROW_COUNT);
        float t = inverse_lerp(min, max, invaders_killed);
        speed = ease_in_quart(MIN_SPEED, MAX_SPEED, t);
    }
    // animation speed adjustment
    {
        const int min = MAX_INVADERS - (INVADER_COLUMN_COUNT * 3);
        const int max = MAX_INVADERS;
        float t = inverse_lerp(min, max, invaders_killed);
        frames_to_flip = ease_in_expo(MIN_FRAMES_TO_FLIP, MAX_FRAMES_TO_FLIP, 1.f - t);
    }
}

void free_invaders() {
    gfx->freeBitmap(invader_frames[0]);
    gfx->freeBitmap(invader_frames[1]);
    for (int i = 0; i < MAX_INVADERS; ++i) {
        spr->freeSprite(invaders[i].sprite);
    }
}
