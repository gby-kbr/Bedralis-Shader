#ifndef WAVE_H
#define WAVE_H

#include "origin/configuration.h"

// Animated water surface, evaluated per pixel (normals) and optionally per vertex (height).
//
// Three layers, each fading out with distance at its own pace so far water stays
// calm instead of shimmering:
//   swell  : long gentle waves, visible far away (also drive the optional vertex waves)
//   medium : the main chop, bent by a slow domain warp so it does not look like a grid
//   detail : fine ripples, only close to the camera
// Plus small fast ripples while it rains.
//
// All wave vectors are integer multiples of 2*PI/64, so the pattern repeats every
// 64 blocks and positions can be wrapped before use. That keeps the sine arguments
// small far from the world origin (float precision) and keeps chunks continuous.

// Height of the long swell in blocks (at WATER_VERTEX_WAVE_SCALE = 1 the amplitude
// is at most 0.085 blocks). Used only when WATER_VERTEX_WAVES is enabled.
float waterWaveHeight(vec2 p, float t) {
  p = p - 64.0 * floor(p / 64.0);
  vec2 ks1 = vec2( 0.393, 0.295);
  vec2 ks2 = vec2(-0.295, 0.491);
  return (0.050 * sin(dot(p, ks1) + 0.9 * t) + 0.035 * sin(dot(p, ks2) - 0.7 * t)) * WATER_VERTEX_WAVE_SCALE;
}

//   p    : world position xz
//   t    : time in seconds
//   rain : 0..1, adds small fast ripples
//   dist : camera distance
vec3 waterNormal(vec2 p, float t, float rain, float dist) {
  p = p - 64.0 * floor(p / 64.0);

  // Swell.
  vec2 ks1 = vec2( 0.393, 0.295);
  vec2 ks2 = vec2(-0.295, 0.491);
  vec2 gSwell = ks1 * (0.050 * cos(dot(p, ks1) + 0.9 * t))
              + ks2 * (0.035 * cos(dot(p, ks2) - 0.7 * t));

  // Slow domain warp for the medium and detail layers.
  vec2 pw = p + 0.9 * vec2(sin(dot(p, vec2( 0.196, 0.098)) + 0.35 * t),
                           cos(dot(p, vec2(-0.098, 0.196)) - 0.28 * t));

  // Medium chop.
  vec2 km1 = vec2( 0.982,  0.687);
  vec2 km2 = vec2(-0.785,  1.277);
  vec2 km3 = vec2( 0.589, -1.963);
  vec2 gMed = km1 * (0.035 * cos(dot(pw, km1) + 1.0 * t))
            + km2 * (0.022 * cos(dot(pw, km2) - 1.3 * t))
            + km3 * (0.014 * cos(dot(pw, km3) + 1.7 * t));

  // Fine detail.
  vec2 kd1 = vec2( 2.258, -0.884);
  vec2 kd2 = vec2(-1.670,  1.866);
  vec2 gDet = kd1 * (0.0065 * cos(dot(pw, kd1) + 2.3 * t))
            + kd2 * (0.0055 * cos(dot(pw, kd2) - 2.0 * t));

  // Rain ripples.
  vec2 kr1 = vec2( 2.749, 0.884);
  vec2 kr2 = vec2(-1.178, 2.651);
  vec2 gRain = (kr1 * cos(dot(p, kr1) + 2.6 * t) + kr2 * cos(dot(p, kr2) - 2.2 * t)) * (0.1 * WATER_RAIN_RIPPLE * rain);

  float fadeSwell  = 1.0 - smoothstep(WATER_SWELL_FADE_START,  WATER_SWELL_FADE_END,  dist);
  float fadeMedium = 1.0 - smoothstep(WATER_WAVE_FADE_START,   WATER_WAVE_FADE_END,   dist);
  float fadeDetail = 1.0 - smoothstep(WATER_DETAIL_FADE_START, WATER_DETAIL_FADE_END, dist);

  vec2 g = gSwell * fadeSwell + (gMed + gRain) * fadeMedium + gDet * fadeDetail;
  g *= WATER_WAVE_STRENGTH;

  return normalize(vec3(-g.x, 1.0, -g.y));
}

#endif
