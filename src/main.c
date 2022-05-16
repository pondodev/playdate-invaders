#include "main.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif

static const struct playdate_sys* sys = NULL;
static const struct playdate_sprite* spr = NULL;
static const struct playdate_graphics* gfx = NULL;
static const struct playdate_sound* snd = NULL;
static const ListManager* lm = NULL;

static List projectiles; 

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
	int player_height;
	gfx->getBitmapData(player.image, NULL, &player_height, NULL, NULL, NULL);
    spr->moveTo(player.sprite, SCREEN_WIDTH / 2, SCREEN_HEIGHT - player_height);

	player.walkSpeed = 2;
	player.runSpeed = 5;

	// GAMER TIME
	pd->display->setRefreshRate(50);

	init_sound_engine(pd);
	play_music();

	lm = get_list_manager();
	projectiles = lm->init();

    sys->setUpdateCallback(update, pd);
}

static ListNodeAction update_projectiles(uint32_t index, void* data) {
	Projectile* proj = (Projectile*)data;
	move_projectile(proj);
	if (proj->y < 0) return kRemove;

	gfx->fillEllipse(proj->x - 2, proj->y - 2, 4, 4, 0, 360, kColorBlack);
	return kNoAction;
}

static int update(void* userdata) {
    PlaydateAPI* pd = userdata;

    // move the sprite around
	{
		PDButtons current, pushed;
		sys->getButtonState(&current, &pushed, NULL);

		int x = 0;

		if (current & kButtonLeft) --x;
		if (current & kButtonRight) ++x;

		int spd = current & kButtonB ? player.runSpeed : player.walkSpeed;
		spr->moveBy(player.sprite, x * spd, 0);

		if (pushed & kButtonA) {
			float x, y;
			spr->getPosition(player.sprite, &x, &y);
			Projectile* new_proj = new_projectile((int)x, (int)y, 5);
			lm->add(&projectiles, new_proj);
		}
	}

	// wrap around screen
	{
		float x, y;
		spr->getPosition(player.sprite, &x, &y);

		if (x < 0) x += SCREEN_WIDTH;
		if (x > SCREEN_WIDTH) x -= SCREEN_WIDTH;
		pd->sprite->moveTo(player.sprite, x, y);
	}

	// sprite go wee
	if (! sys->isCrankDocked() && fabsf(sys->getCrankChange()) > 1.f) {
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

	lm->iterate(&projectiles, update_projectiles);

    sys->drawFPS(0,0);

    return 1;
}
