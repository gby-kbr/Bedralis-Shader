#ifndef WAVE_H
#define WAVE_H

#include "origin/configuration.h"

// Animated water surface normal, evaluated per pixel. There is no vertex
// displacement on purpose: moving water vertices would open cracks at
// chunk borders and along shores.
//
// The wave directions are integer multiples of 2*PI/64, so the pattern repeats
// every 64 blocks and the position can be wrapped before use. That keeps the
// sine arguments small even far from the world origin (float precision).
//   p    : world position xz
//   t    : time in seconds
//   rain : 0..1, adds small fast ripples
//   dist : camera distance, waves fade out far away
vec3 waterNormal(vec2 p, float t, float rain, float dist) {
  p = p - 64.0 * floor(p / 64.0);

  vec2 k1 = vec2( 0.982,  0.687);
  vec2 k2 = vec2(-0.785,  1.277);
  vec2 k3 = vec2( 0.589, -1.963);
  vec2 k4 = vec2( 2.749,  0.884);   // rain ripples
  vec2 k5 = vec2(-1.178,  2.651);   // rain ripples

  vec2 g = k1 * (0.050 * cos(dot(p, k1) + 1.0 * t))
         + k2 * (0.035 * cos(dot(p, k2) - 1.3 * t))
         + k3 * (0.025 * cos(dot(p, k3) + 1.7 * t));

  g += (k4 * cos(dot(p, k4) + 2.6 * t) + k5 * cos(dot(p, k5) - 2.2 * t)) * (0.1 * WATER_RAIN_RIPPLE * rain);

  float fade = 1.0 - smoothstep(WATER_WAVE_FADE_START, WATER_WAVE_FADE_END, dist);
  g *= WATER_WAVE_STRENGTH * fade;

  return normalize(vec3(-g.x, 1.0, -g.y));
}

#endif
