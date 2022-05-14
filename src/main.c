#include "main.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif

static const struct playdate_sys* sys = NULL;
static const struct playdate_sprite* spr = NULL;
static const struct playdate_graphics* gfx = NULL;
static const struct playdate_sound* snd = NULL;

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
	sys = pd->system;
	spr = pd->sprite;
	gfx = pd->graphics;
	snd = pd->sound;

	// initialise player stuffs
    const char* err;
    player.image = gfx->loadBitmap("Images/player_sprite", &err);
    if (player.image == NULL) {
        sys->error("failed to load player bitmap: %s", err);
	}
	player.rotatedImage = NULL;

    player.sprite = spr->newSprite();
    spr->addSprite(player.sprite);
    spr->setImage(player.sprite, player.image, kBitmapUnflipped);
    spr->moveTo(player.sprite, 200, 120);

	player.walkSpeed = 2;
	player.runSpeed = 5;

	// GAMER TIME
	pd->display->setRefreshRate(50);

	init_sound_engine(pd);
	play_music();

    sys->setUpdateCallback(update, pd);
}

static int update(void* userdata) {
    PlaydateAPI* pd = userdata;

    // move the sprite around
	{
		PDButtons current;
		sys->getButtonState(&current, NULL, NULL);

		int x = 0;
		int y = 0;

		if (current & kButtonUp) --y;
		if (current & kButtonDown) ++y;
		if (current & kButtonLeft) --x;
		if (current & kButtonRight) ++x;

		int spd = current & kButtonB ? player.runSpeed : player.walkSpeed;
		spr->moveBy(player.sprite, x * spd, y * spd);
	}

	// wrap around screen
	{
		float x, y;
		spr->getPosition(player.sprite, &x, &y);

		if (x < 0) x += SCREEN_WIDTH;
		if (x > SCREEN_WIDTH) x -= SCREEN_WIDTH;
		if (y < 0) y += SCREEN_HEIGHT;
		if (y > SCREEN_HEIGHT) y -= SCREEN_HEIGHT;
		pd->sprite->moveTo(player.sprite, x, y);
	}

	// sprite go wee
	if (! sys->isCrankDocked()) {
		// this is actually really unperformant. should be only rotating the
		// bitmap when there's a change in the crank angle
		float angle = sys->getCrankAngle();
		LCDBitmap* old = player.rotatedImage;
		player.rotatedImage = gfx->rotatedBitmap(player.image, angle, 1, 1, NULL);
		spr->setImage(player.sprite, player.rotatedImage, kBitmapUnflipped);

		if (old != NULL) {
			gfx->freeBitmap(old);
		}
	}

    // draw stuff
    gfx->clear(kColorWhite);
    spr->drawSprites();

    sys->drawFPS(0,0);

    return 1;
}
