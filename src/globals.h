#ifndef GLOBALS_H
#define GLOBALS_H

#include "pd_api.h"

#include "list.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

typedef enum CollisionTag {
    kTagNone        = 0,
    kTagPlayer      = 1,
    kTagEnemy       = 2,
    kTagProjectile  = 3,
} CollisionTag;

extern const PlaydateAPI* pd;
extern const struct playdate_sys* sys;
extern const struct playdate_sprite* spr;
extern const struct playdate_graphics* gfx;
extern const struct playdate_sound* snd;
extern const struct playdate_sound_sequence* seq;
extern const struct playdate_sound_synth* synth;

extern const ListManager lm;
extern int game_running;

#endif