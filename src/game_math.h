#ifndef GAME_MATH_H
#define GAME_MATH_H

#include <stdlib.h>
#include <math.h>

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

// limiting
float min(float a, float b);
float max(float a, float b);
float clamp(float min_val, float max_val, float val);

// interpolation
float lerp(float a, float b, float t);
float inverse_lerp(float a, float b, float val);
float remap(float in_min, float in_max, float out_min, float out_max, float val);
float ease_in_quart(float a, float b, float t);
float ease_in_expo(float a, float b, float t);

// random
float frand(float min, float max);
int irand(int min, int max);

#endif
