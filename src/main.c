#include "main.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif

typedef struct PlayerInfo {
	LCDBitmap* image;
	LCDBitmap* rotatedImage;
	LCDSprite* sprite;
	int walkSpeed;
	int runSpeed;
} PlayerInfo;

PlayerInfo player;

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    (void)arg; // arg is currently only used for event = kEventKeyPressed

    if (event == kEventInit) init(pd);

    return 0;
}

static void init(PlaydateAPI* pd) {
	// initialise player stuffs
    const char* err;
    player.image = pd->graphics->loadBitmap("Images/player_sprite", &err);
    if (player.image == NULL) {
        pd->system->error("failed to load player bitmap: %s", err);
	}
	player.rotatedImage = NULL;

    player.sprite = pd->sprite->newSprite();
    pd->sprite->addSprite(player.sprite);
    pd->sprite->setImage(player.sprite, player.image, kBitmapUnflipped);
    pd->sprite->moveTo(player.sprite, 200, 120);

	player.walkSpeed = 2;
	player.runSpeed = 5;

	// GAMER TIME
	pd->display->setRefreshRate(50);

    pd->system->setUpdateCallback(update, pd);
}

static int update(void* userdata) {
    PlaydateAPI* pd = userdata;

    // move the sprite around
	{
		PDButtons current;
		pd->system->getButtonState(&current, NULL, NULL);

		int x = 0;
		int y = 0;

		if (current & kButtonUp) --y;
		if (current & kButtonDown) ++y;
		if (current & kButtonLeft) --x;
		if (current & kButtonRight) ++x;

		int spd = current & kButtonB ? player.runSpeed : player.walkSpeed;
		pd->sprite->moveBy(player.sprite, x * spd, y * spd);
	}

	// wrap around screen
	{
		float x, y;
		pd->sprite->getPosition(player.sprite, &x, &y);

		if (x < 0) x += SCREEN_WIDTH;
		if (x > SCREEN_WIDTH) x -= SCREEN_WIDTH;
		if (y < 0) y += SCREEN_HEIGHT;
		if (y > SCREEN_HEIGHT) y -= SCREEN_HEIGHT;
		pd->sprite->moveTo(player.sprite, x, y);
	}

	// sprite go wee
	if (! pd->system->isCrankDocked()) {
		// this is actually really unperformant. should be only rotating the
		// bitmap when there's a change in the crank angle
		float angle = pd->system->getCrankAngle();
		LCDBitmap* old = player.rotatedImage;
		player.rotatedImage = pd->graphics->rotatedBitmap(player.image, angle, 1, 1, NULL);
		pd->sprite->setImage(player.sprite, player.rotatedImage, kBitmapUnflipped);

		if (old != NULL) {
			pd->graphics->freeBitmap(old);
		}
	}

    // draw stuff
    pd->graphics->clear(kColorWhite);
    pd->sprite->drawSprites();

    pd->system->drawFPS(0,0);

    return 1;
}
