#ifndef GAME_MATH_H
#define GAME_MATH_H

float lerp(float a, float b, float t);
float inverse_lerp(float a, float b, float val);
float remap(float in_min, float in_max, float out_min, float out_max, float val);

#endif
