#ifndef DOOTER_H
#define DOOTER_H

#include "pd_api.h"

void init_music(PlaydateAPI* pd);
void free_music(void);
void play_music(void);
void pause_music(void);

#endif
