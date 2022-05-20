#ifndef SOUND_EFFECTS_H
#define SOUND_EFFECTS_H

#include "pd_api.h"

#include "game_math.h"

void init_sound_effects(PlaydateAPI* pd);
void free_sound_effects(void);
void mute_sound_effects(void);
void unmute_sound_effects(void);
void play_projectile_sound(void);
void play_reload_sound(float percent);

#endif
