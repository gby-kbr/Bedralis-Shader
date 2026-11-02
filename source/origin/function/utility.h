#ifndef UTILITY_H
#define UTILITY_H

#include "origin/configuration.h"

float degToRad(float deg) {
  return deg * (PI / 180.0);
}

// Rotates a 2D vector by an angle in radians.
// Written with plain scalar math instead of mat2 so it behaves the same on
// GLSL, HLSL and Metal (the multiplatform profile builds all three).
vec2 rotate2(vec2 v, float a) {
  float c = cos(a);
  float s = sin(a);
  return vec2(c*v.x - s*v.y, s*v.x + c*v.y);
}

#endif
