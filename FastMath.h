#pragma once

#include <cstdint>

// Taken from SimplexNoise library
/**
 * Computes the largest integer value not greater than the float one
 *
 * This method is faster than using (int32_t)std::floor(fp).
 *
 * I measured it to be approximately twice as fast:
 *  float:  ~18.4ns instead of ~39.6ns on an AMD APU),
 *  double: ~20.6ns instead of ~36.6ns on an AMD APU),
 * Reference: http://www.codeproject.com/Tips/700780/Fast-floor-ceiling-functions
 *
 * @param[in] fp    float input value
 *
 * @return largest integer value not greater than fp
 */

static inline int32_t fastfloor(float fp) {
    int32_t i = static_cast<int32_t>(fp);
    return (fp < i) ? (i - 1) : i;
}

// Based on fastfloor() from SimplexNoise library
static inline int32_t fastceil(float fp) {
    int32_t i = static_cast<int32_t>(fp);
    return (fp > i) ? (i + 1) : i;
}

static inline float clamp(float min, float max, float val) {
    return val < min ? min : (val > max ? max : val);
}

static inline float loop(float min, float max, float val) {
    float dif = max - min;
    while (val <  min) { val += dif; }
    while (val >= dif) { val -= dif; }
    return val;
}