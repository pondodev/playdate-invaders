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
	LCDBitmap*	image;
	LCDSprite*	sprite;
	int			walk_speed;
	int			run_speed;
	float		ammo_percent;
	float		ammo_reload_rate;		// how much % 1 degree of crank rotation reloads
	float		ammo_consumption_rate;	// how much % firing once uses up
} PlayerInfo;

PlayerInfo player;

LCDBitmap* projectile_image = NULL;

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

    player.sprite = spr->newSprite();
    spr->addSprite(player.sprite);
    spr->setImage(player.sprite, player.image, kBitmapUnflipped);
	int player_height;
	gfx->getBitmapData(player.image, NULL, &player_height, NULL, NULL, NULL);
    spr->moveTo(player.sprite, SCREEN_WIDTH / 2, SCREEN_HEIGHT - player_height);

	player.walk_speed = 2;
	player.run_speed = 5;
	player.ammo_percent = 100.f;
	player.ammo_reload_rate = 0.1f;
	player.ammo_consumption_rate = 10.f;

	projectile_image = gfx->loadBitmap("Images/projectile", &err);
	if (projectile_image == NULL) {
		sys->error("failed to load projectile bitmap: %s", err);
	}

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

	return kNoAction;
}

static ListNodeAction draw_projectiles(uint32_t index, void* data) {
	Projectile* proj = (Projectile*)data;
	if (++proj->frames_since_flip == proj->frames_to_flip) {
		proj->frames_since_flip = 0;
		proj->flipped = !proj->flipped;
	}

	LCDBitmapFlip flip = proj->flipped ? kBitmapFlippedX : kBitmapUnflipped;
	gfx->drawBitmap(projectile_image, proj->x, proj->y, flip);

	return kNoAction;
}

static int update(void* userdata) {
    PlaydateAPI* pd = userdata;

	// ammo reloading
	if (! sys->isCrankDocked()) {
		float angle_delta = sys->getCrankChange();
		if (angle_delta > 0.f) {
			player.ammo_percent += angle_delta * player.ammo_reload_rate;
			player.ammo_percent = player.ammo_percent < 100.f ? player.ammo_percent : 100.f;
		}
	}

    // move the sprite around
	{
		PDButtons current, pushed;
		sys->getButtonState(&current, &pushed, NULL);

		int x = 0;

		if (current & kButtonLeft) --x;
		if (current & kButtonRight) ++x;

		int spd = current & kButtonB ? player.run_speed : player.walk_speed;
		spr->moveBy(player.sprite, x * spd, 0);

		if (pushed & kButtonA && player.ammo_percent >= player.ammo_consumption_rate) {
			player.ammo_percent -= player.ammo_consumption_rate;

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

	lm->iterate(&projectiles, update_projectiles);

    // draw stuff
    gfx->clear(kColorWhite);
    spr->drawSprites();

	lm->iterate(&projectiles, draw_projectiles);

	// ammo display
	{
		int center_x = SCREEN_WIDTH / 2;
		int center_y = SCREEN_HEIGHT / 2;
		int width = 50;
		int height = 50;
		int inner_width = remap(0, 100, 0, width, player.ammo_percent);
		int inner_height = remap(0, 100, 0, height, player.ammo_percent);

		gfx->drawEllipse(center_x - width / 2, center_y - height / 2, width, height,  1, 0, 360, kColorBlack);
		gfx->fillEllipse(center_x - inner_width / 2, center_y - inner_height / 2, inner_width, inner_height, 0, 360, kColorBlack);
	}

    sys->drawFPS(0,0);

    return 1;
}
