#ifndef SAVE_H
#define SAVE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "pd_api.h"

#include "globals.h"

typedef struct SaveData {
    int music_enabled;
    int sound_effects_enabled;
} SaveData;

void save_data(SaveData data);
SaveData load_data(void);

#endif
