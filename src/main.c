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
	Vec2		position;
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

	player.position.x = SCREEN_WIDTH / 2;
	player.position.y = SCREEN_HEIGHT - player_height;
    spr->moveTo(player.sprite, player.position.x, player.position.y);

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
	if (proj->position.y < 0) return kRemove;

	return kNoAction;
}

static ListNodeAction draw_projectiles(uint32_t index, void* data) {
	Projectile* proj = (Projectile*)data;
	if (++proj->frames_since_flip == proj->frames_to_flip) {
		proj->frames_since_flip = 0;
		proj->flipped = !proj->flipped;
	}

	LCDBitmapFlip flip = proj->flipped ? kBitmapFlippedX : kBitmapUnflipped;
	gfx->drawBitmap(projectile_image, proj->position.x, proj->position.y, flip);

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
	PDButtons current, pushed;
	sys->getButtonState(&current, &pushed, NULL);

	int move_vel = 0;

	if (current & kButtonLeft) --move_vel;
	if (current & kButtonRight) ++move_vel;

	int spd = current & kButtonB ? player.run_speed : player.walk_speed;
	move_vel *= spd;
	player.position.x += move_vel;

	// screen wrapping
	if (player.position.x < 0) player.position.x += SCREEN_WIDTH;
	else if (player.position.x > SCREEN_WIDTH) player.position.x -= SCREEN_WIDTH;

	spr->moveTo(player.sprite, player.position.x, player.position.y);

	if (pushed & kButtonA && player.ammo_percent >= player.ammo_consumption_rate) {
		player.ammo_percent -= player.ammo_consumption_rate;
		Projectile* new_proj = new_projectile(player.position.x, player.position.y, 5);
		lm->add(&projectiles, new_proj);
	}

	lm->iterate(&projectiles, update_projectiles);

    // draw stuff
    gfx->clear(kColorWhite);
    spr->drawSprites();

	lm->iterate(&projectiles, draw_projectiles);

	// ammo display
	{
		int offset = 12;
		int width = 4;
		int height = 20;
		int fill_height = remap(0, 100, 0, 20, player.ammo_percent);

		Vec2 left_display = (Vec2) {
			.x = player.position.x - offset - width,
			.y = player.position.y - height / 2
		};
		Vec2 right_display = (Vec2) {
			.x = player.position.x + offset,
			.y = left_display.y
		};

		Vec2 left_display_fill = (Vec2) {
			.x = left_display.x,
			.y = left_display.y + (height - fill_height)
		};
		Vec2 right_display_fill = (Vec2) {
			.x = right_display.x,
			.y = right_display.y + (height - fill_height)
		};

		gfx->drawRect(left_display.x, left_display.y, width, height, kColorBlack);
		gfx->drawRect(right_display.x, right_display.y, width, height, kColorBlack);
		gfx->fillRect(left_display_fill.x, left_display_fill.y, width, fill_height, kColorBlack);
		gfx->fillRect(right_display_fill.x, right_display_fill.y, width, fill_height, kColorBlack);
	}

    sys->drawFPS(0,0);

    return 1;
}
