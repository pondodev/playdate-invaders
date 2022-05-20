#ifndef GAME_MATH_H
#define GAME_MATH_H

#include <stdlib.h>

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

float lerp(float a, float b, float t);
float inverse_lerp(float a, float b, float val);
float remap(float in_min, float in_max, float out_min, float out_max, float val);
float frand(float min, float max);
int irand(int min, int max);

#endif
