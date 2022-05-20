#include "main.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif

static const struct playdate_sys* sys = NULL;
static const struct playdate_sprite* spr = NULL;
static const struct playdate_graphics* gfx = NULL;
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

typedef struct PlayerInput {
    int			firing;
    int			running;
    int			horizontal_axis;
    float		crank_delta;
} PlayerInput;

typedef struct MenuItems {
    PDMenuItem* music_enabled;
    PDMenuItem* sound_effects_enabled;
} MenuItems;

PlayerInfo player;
PlayerInput input;
MenuItems menu;

LCDBitmap* projectile_image = NULL;

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    (void)arg; // arg is currently only used for event = kEventKeyPressed

    if (event == kEventInit) init(pd);
    if (event == kEventTerminate) game_terminated();

    return 0;
}

static void init(PlaydateAPI* pd) {
    sys = pd->system;
    spr = pd->sprite;
    gfx = pd->graphics;

    // menu items
    menu.music_enabled = sys->addCheckmarkMenuItem("music", 1, on_menu_music_change, NULL);
    menu.sound_effects_enabled = sys->addCheckmarkMenuItem("sound fx", 1, on_menu_sound_effects_change, NULL);

    // player defaults
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

    // input defaults
    input.firing = 0;
    input.running = 0;
    input.horizontal_axis = 0;
    input.crank_delta = 0.f;

    // projectiles
    projectile_image = gfx->loadBitmap("Images/projectile", &err);
    if (projectile_image == NULL) {
        sys->error("failed to load projectile bitmap: %s", err);
    }

    lm = get_list_manager();
    projectiles = lm->init();

    // other
    init_music(pd);
    play_music();

    init_sound_effects(pd);

    sys->setUpdateCallback(update, pd);
}

static int update(void* userdata) {
    PlaydateAPI* pd = userdata;

    process_input();
    process_player();
    lm->iterate(&projectiles, update_projectiles);
    draw();

    return 1;
}

static void process_input() {
    input.horizontal_axis = 0;
    input.crank_delta = 0.f;

    PDButtons current, pushed;
    sys->getButtonState(&current, &pushed, NULL);

    input.firing = pushed & kButtonA;

    input.running = current & kButtonB;

    if (current & kButtonLeft) --input.horizontal_axis;
    if (current & kButtonRight) ++input.horizontal_axis;

    if (! sys->isCrankDocked()) {
        float angle_delta = sys->getCrankChange();
        if (angle_delta > 0.1f) input.crank_delta = angle_delta;
    }
}

static void process_player() {
    if (input.crank_delta > 0.1f) {
        player.ammo_percent += input.crank_delta * player.ammo_reload_rate;
        player.ammo_percent = player.ammo_percent < 100.f ? player.ammo_percent : 100.f;

        // this just stops the sound from playing too long after the player lets go of
        // the crank, or when they accidentally nudge it
        if (input.crank_delta > 1.f)
            play_reload_sound(player.ammo_percent);
    }

    int speed_multiplier = input.running ? player.run_speed : player.walk_speed;
    player.position.x += input.horizontal_axis * speed_multiplier;

    // screen wrapping
    if (player.position.x < 0) player.position.x += SCREEN_WIDTH;
    else if (player.position.x > SCREEN_WIDTH) player.position.x -= SCREEN_WIDTH;

    spr->moveTo(player.sprite, player.position.x, player.position.y);

    if (input.firing && player.ammo_percent >= player.ammo_consumption_rate) {
        play_projectile_sound();

        player.ammo_percent -= player.ammo_consumption_rate;
        Projectile* new_proj = new_projectile(player.position.x, player.position.y, 5);
        lm->add(&projectiles, new_proj);
    }
}

static void draw() {
    gfx->clear(kColorWhite);
    spr->drawSprites();

    lm->iterate(&projectiles, draw_projectiles);

    // ammo display
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

    sys->drawFPS(0,0);
}

static void game_terminated() {
    // probably don't need to, but it's just polite :)
    lm->destroy(&projectiles);
    spr->freeSprite(player.sprite);
    gfx->freeBitmap(player.image);
    gfx->freeBitmap(projectile_image);
    free_music();
    free_sound_effects();
}

static void on_menu_music_change(void* userdata) {
    int playing = sys->getMenuItemValue(menu.music_enabled);
    playing ? play_music() : pause_music();
}

static void on_menu_sound_effects_change(void* userdata) {
    int muted = ! sys->getMenuItemValue(menu.sound_effects_enabled);
    muted ? mute_sound_effects() : unmute_sound_effects();
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
