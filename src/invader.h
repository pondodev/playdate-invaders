#ifndef INVADER_H
#define INVADER_H

#include "pd_api.h"
#include "game_math.h"
#include "globals.h"

typedef struct Invader {
    LCDSprite* sprite;
    Vec2 position;
    int alive;
} Invader;

typedef void (*GameLostCallback)(void);

void init_invader_data(PlaydateAPI* playdate, GameLostCallback callback);
void update_invaders(void);
void free_invaders(void);

#endif
