#include "game_math.h"

float min(float a, float b) {
    return a < b ? a : b;
}

float max(float a, float b) {
    return a > b ? a : b;
}

float clamp(float min_val, float max_val, float val) {
    return min(max_val, max(min_val, val));
}

float lerp(float a, float b, float t) {
    t = clamp(0.f, 1.f, t);
    return a + t * (b - a);
}

float inverse_lerp(float a, float b, float val) {
    return clamp(0.f, 1.f, (val - a) / (b - a));
}

float remap(float in_min, float in_max, float out_min, float out_max, float val) {
    float t = inverse_lerp(in_min, in_max, val);
    return lerp(out_min, out_max, t);
}

float ease_in_quart(float a, float b, float t) {
    return lerp(a, b, t * t * t * t);
}

float ease_in_expo(float a, float b, float t) {
    return lerp(a, b, powf(2, 10 * t - 10));
}

float frand(float min, float max) {
    return (((float)rand() / (float)(RAND_MAX)) * max) + min;
}

int irand(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}
