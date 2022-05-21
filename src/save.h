#ifndef SAVE_H
#define SAVE_H

#include "pd_api.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct SaveData {
    int music_enabled;
    int sound_effects_enabled;
} SaveData;

void save_data(PlaydateAPI* pd, SaveData data);
SaveData load_data(PlaydateAPI* pd);

#endif
