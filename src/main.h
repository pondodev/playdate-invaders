#ifndef MAIN_H
#define MAIN_H

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pd_api.h"
#include "dooter.h"
#include "list.h"
#include "projectile.h"
#include "game_math.h"

static void init(PlaydateAPI* pd);
static int update(void* userdata);

#endif

