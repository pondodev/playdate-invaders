#ifndef INVADER_H
#define INVADER_H

#include "pd_api.h"

#include "globals.h"
#include "game_math.h"

typedef struct Invader {
    LCDSprite* sprite;
    Vec2 position;
    int alive;
} Invader;

typedef void (*GameLostCallback)(void);

void init_invader_data(GameLostCallback callback);
void reset_invaders(void);
void update_invaders(void);
void on_invader_kill(void);
void free_invaders(void);

#endif
