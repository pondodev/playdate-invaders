#include "main.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif

const PlaydateAPI* pd = NULL;
const struct playdate_sys* sys = NULL;
const struct playdate_sprite* spr = NULL;
const struct playdate_graphics* gfx = NULL;
const struct playdate_sound* snd = NULL;
const struct playdate_sound_sequence* seq = NULL;
const struct playdate_sound_synth* synth = NULL;

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

SaveData player_save_data;
PlayerInfo player;
PlayerInput input;
MenuItems menu;

LCDBitmap* projectile_image = NULL;

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg) {
    (void)arg; // arg is currently only used for event = kEventKeyPressed

    if (event == kEventInit) init(pd);
    if (event == kEventTerminate) game_terminated(pd);

    return 0;
}

static void init(PlaydateAPI* playdate) {
    pd = playdate;
    sys = playdate->system;
    spr = playdate->sprite;
    gfx = playdate->graphics;
    snd = playdate->sound;
    seq = playdate->sound->sequence;
    synth = playdate->sound->synth;

    player_save_data = load_data();

    // player defaults
    const char* err;
    player.image = gfx->loadBitmap("Images/player_sprite", &err);
    if (player.image == NULL) {
        sys->error("failed to load player bitmap: %s", err);
    }

    player.sprite = spr->newSprite();
    spr->setTag(player.sprite, kTagPlayer);
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

    // invaders
    init_invader_data(game_lost);

    // input defaults
    input.firing = 0;
    input.running = 0;
    input.horizontal_axis = 0;
    input.crank_delta = 0.f;

    // projectiles
    init_projectiles();
    lm = get_list_manager();
    projectiles = lm->init();

    // sound
    init_music();
    init_sound_effects();

    // menu items
    menu.music_enabled = sys->addCheckmarkMenuItem("music", player_save_data.music_enabled, on_menu_music_change, NULL);
    menu.sound_effects_enabled = sys->addCheckmarkMenuItem("sound fx", player_save_data.sound_effects_enabled, on_menu_sound_effects_change, NULL);
    on_menu_music_change(NULL);
    on_menu_sound_effects_change(NULL);

    sys->setUpdateCallback(update, NULL);
}

static int update(void* userdata) {
    process_input();
    process_player();
    lm->iterate(&projectiles, update_projectiles);
    draw();
    update_invaders();

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

static void game_terminated(PlaydateAPI* pd) {
    // probably don't need to, but it's just polite :)
    lm->destroy(&projectiles);
    spr->freeSprite(player.sprite);
    gfx->freeBitmap(player.image);
    gfx->freeBitmap(projectile_image);
    free_music();
    free_sound_effects();
    free_invaders();

    save_data(player_save_data);
}

static void on_menu_music_change(void* userdata) {
    int playing = sys->getMenuItemValue(menu.music_enabled);
    playing ? play_music() : pause_music();
    player_save_data.music_enabled = playing;
}

static void on_menu_sound_effects_change(void* userdata) {
    int enabled = sys->getMenuItemValue(menu.sound_effects_enabled);
    enabled ? unmute_sound_effects() : mute_sound_effects();
    player_save_data.sound_effects_enabled = enabled;
}

static ListNodeAction update_projectiles(uint32_t index, void* data) {
    Projectile* proj = (Projectile*)data;
    update_projectile(proj);
    if (proj->position.y < 0) {
        spr->freeSprite(proj->sprite);
        return kRemove;
    }

    return kNoAction;
}

static void game_lost() {
    sys->logToConsole("TODO: implement game lose state!");
}
