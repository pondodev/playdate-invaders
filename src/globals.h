#ifndef GLOBALS_H
#define GLOBALS_H

#include "pd_api.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

enum CollisionTag {
    kTagPlayer      = 0,
    kTagEnemy       = 1,
    kTagProjectile  = 2,
};

extern const PlaydateAPI* pd;
extern const struct playdate_sys* sys;
extern const struct playdate_sprite* spr;
extern const struct playdate_graphics* gfx;
extern const struct playdate_sound* snd;
extern const struct playdate_sound_sequence* seq;
extern const struct playdate_sound_synth* synth;

#endif