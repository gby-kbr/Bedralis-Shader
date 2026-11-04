#ifndef CELESTIAL_H
#define CELESTIAL_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"

// The engine places the sun/moon quads and picks the moon phase cell in the
// atlas itself, so the shader only controls their color and brightness.

// Sun disc tint: warm near the horizon, near white when high.
vec3 sunDiscTint(environment env) {
  float high = smoothstep(0.0, 0.45, env.dayFactor);
  vec3 tint = mix(SUN_TINT_LOW, SUN_TINT_HIGH, high);
  return tint * (1.0 - SUN_RAIN_DIM * env.rainFactor);
}

// Moon disc tint: cool and slightly dimmed in rain.
vec3 moonDiscTint(environment env) {
  return MOON_TINT * (1.0 - MOON_RAIN_DIM * env.rainFactor);
}

// Tint for a sun/moon quad vertex. Both bodies use the same material, so the
// body is told apart by which side of the sky the vertex is on.
//   viewDir : normalized direction from the camera to the vertex
vec3 celestialBodyTint(environment env, vec3 viewDir) {
  float sunSide = smoothstep(-0.10, 0.10, dot(viewDir, env.sunDir));
  return mix(moonDiscTint(env), sunDiscTint(env), sunSide);
}

// Moonlight strength from the MoonPhase uniform (x = phase index 0 to 7,
// 0 = full moon, 4 = new moon). Not used for the disc itself (the atlas already
// shows the phase); meant for night lighting in Phase 05.
float moonlightFromPhase(float moonPhase) {
  float a = 2.0 * PI * moonPhase / 8.0;
  float full = 0.5 + 0.5 * cos(a);   // 1 at full moon, 0 at new moon
  return mix(MOON_NEW_LIGHT, 1.0, full);
}

// Star brightness multiplier: only at night, fading out in rain.
float starVisibility(environment env) {
  float vis = smoothstep(0.20, 0.90, env.nightFactor);
  return vis * (1.0 - STAR_RAIN_FADE * env.rainFactor);
}

#endif
