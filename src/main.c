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

typedef struct PlayerInput {
    int			firing;
    int			running;
    int			horizontal_axis;
    float		crank_delta;
} PlayerInput;

PlayerInfo player;
PlayerInput input;

LCDBitmap* projectile_image = NULL;

SoundChannel* sound_effects = NULL;
PDSynth* projectile_sound = NULL;

PDMenuItem* menu_music_enabled = NULL;

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
    snd = pd->sound;

    // menu items
    menu_music_enabled = sys->addCheckmarkMenuItem("music", 1, on_music_menu_change, NULL);

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

    input.firing = 0;
    input.running = 0;
    input.horizontal_axis = 0;
    input.crank_delta = 0.f;

    projectile_image = gfx->loadBitmap("Images/projectile", &err);
    if (projectile_image == NULL) {
        sys->error("failed to load projectile bitmap: %s", err);
    }

    init_sound_engine(pd);
    play_music();

    lm = get_list_manager();
    projectiles = lm->init();

    // TODO: my god this audio stuff needs some refactoring
    sound_effects = snd->channel->newChannel();
    projectile_sound = snd->synth->newSynth();
    snd->channel->addSource(sound_effects, (SoundSource*)projectile_sound);
    snd->channel->setVolume(sound_effects, 0.1f);
    snd->addChannel(sound_effects);

    snd->synth->setWaveform(projectile_sound, kWaveformSawtooth);
    snd->synth->setAttackTime(projectile_sound, 0.f);
    snd->synth->setDecayTime(projectile_sound, 0.f);
    snd->synth->setSustainLevel(projectile_sound, 1.f);
    snd->synth->setReleaseTime(projectile_sound, 0.1f);
    BitCrusher* crusher = snd->effect->bitcrusher->newBitCrusher();
    snd->effect->bitcrusher->setUndersampling(crusher, 0.75f);
    PDSynthLFO* projectile_sound_lfo = snd->lfo->newLFO(kLFOTypeSawtoothDown);
    snd->lfo->setRetrigger(projectile_sound_lfo, 1);
    snd->lfo->setRate(projectile_sound_lfo, 2.5f);
    snd->channel->addEffect(sound_effects, (SoundEffect*)crusher);
    snd->synth->setFrequencyModulator(projectile_sound, (PDSynthSignalValue*)projectile_sound_lfo);

    sys->setUpdateCallback(update, pd);
}

static void on_music_menu_change(void* userdata) {
    int playing = sys->getMenuItemValue(menu_music_enabled);
    playing ? play_music() : pause_music();
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
    if (input.crank_delta > 0.f) {
        player.ammo_percent += input.crank_delta * player.ammo_reload_rate;
        player.ammo_percent = player.ammo_percent < 100.f ? player.ammo_percent : 100.f;
    }

    int speed_multiplier = input.running ? player.run_speed : player.walk_speed;
    player.position.x += input.horizontal_axis * speed_multiplier;

    // screen wrapping
    if (player.position.x < 0) player.position.x += SCREEN_WIDTH;
    else if (player.position.x > SCREEN_WIDTH) player.position.x -= SCREEN_WIDTH;

    spr->moveTo(player.sprite, player.position.x, player.position.y);

    if (input.firing && player.ammo_percent >= player.ammo_consumption_rate) {
        snd->synth->playNote(projectile_sound, 700.f, 1.f, 0.1f, 0);

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
}

static void game_terminated() {
    // probably don't need to, but it's just polite :)
    lm->destroy(&projectiles);
    spr->freeSprite(player.sprite);
    gfx->freeBitmap(player.image);
    gfx->freeBitmap(projectile_image);
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
