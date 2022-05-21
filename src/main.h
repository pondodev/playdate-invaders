#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pd_api.h"

#include "globals.h"
#include "dooter.h"
#include "list.h"
#include "projectile.h"
#include "game_math.h"
#include "sound_effects.h"
#include "save.h"
#include "invader.h"

// game loop functions
static void init(PlaydateAPI* pd);
static int update(void* userdata);
static void process_input(void);
static void process_player(void);
static void draw(void);
static void game_terminated(PlaydateAPI* pd);

// menu callbacks
static void on_menu_music_change(void* userdata);
static void on_menu_sound_effects_change(void* userdata);

// list callbacks
static ListNodeAction update_projectiles(uint32_t index, void* data);

// game state callbacks
static void game_lost(void);

#endif
