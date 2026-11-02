#ifndef SKY_H
#define SKY_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"

vec3 skyDesaturate(vec3 c, float amount) {
  float l = dot(c, vec3(0.299, 0.587, 0.114));
  return mix(c, vec3(l, l, l), amount);
}

// Overworld zenith color for the current time and weather.
vec3 overworldSkyZenith(environment env) {
  vec3 c = mix(SKY_NIGHT_ZENITH, SKY_DAY_ZENITH, env.daylight);
  c = mix(c, SKY_DAWN_ZENITH, env.dawnFactor);
  c = mix(c, SKY_DUSK_ZENITH, env.duskFactor);

  // Rain: flat overcast, slightly darker overhead.
  vec3 rainCol = mix(SKY_RAIN_NIGHT, SKY_RAIN_DAY, env.daylight) * 0.85;
  return mix(c, rainCol, env.rainFactor * SKY_RAIN_STRENGTH);
}

// Overworld horizon color for the current time and weather.
vec3 overworldSkyHorizon(environment env) {
  vec3 c = mix(SKY_NIGHT_HORIZON, SKY_DAY_HORIZON, env.daylight);
  c = mix(c, SKY_DAWN_HORIZON, env.dawnFactor);
  c = mix(c, SKY_DUSK_HORIZON, env.duskFactor);

  // Rain: horizon is a little lighter than the zenith.
  vec3 rainCol = mix(SKY_RAIN_NIGHT, SKY_RAIN_DAY, env.daylight) * 1.05;
  return mix(c, rainCol, env.rainFactor * SKY_RAIN_STRENGTH);
}

// Final sky color for one dome vertex.
//   horizonT : 0 at the zenith, 1 at the horizon (vanilla a_color0.x)
//   viewDir  : normalized direction from the camera to the vertex
//   vanillaSky / vanillaFog : the engine's own SkyColor / FogColor
vec3 calculateSkyColor(environment env, float horizonT, vec3 viewDir, vec3 vanillaSky, vec3 vanillaFog) {
  // The End has its own sky (EndSky material, reworked in Phase 10).
  if (env.isEnd) {
    return vanillaSky;
  }

  // The Nether has no real sky: keep the engine fog color, a bit darker and less saturated.
  if (env.isNether) {
    return skyDesaturate(vanillaFog, NETHER_SKY_DESATURATE) * NETHER_SKY_DARKEN;
  }

  float h = clamp(horizonT, 0.0, 1.0);
  h = h*h*(3.0 - 2.0*h);

  vec3 c = mix(overworldSkyZenith(env), overworldSkyHorizon(env), h);

  // Warm glow toward the sun at dawn and dusk, strongest near the horizon.
  float sunSide = clamp(dot(viewDir, env.sunDir), 0.0, 1.0);
  float glow = sunSide*sunSide * (env.dawnFactor + env.duskFactor) * h;
  glow *= 1.0 - env.rainFactor;
  c += SKY_SUN_GLOW * glow * SKY_GLOW_STRENGTH;

  // Keep a little of the engine colors so biome fog tint still shows.
  vec3 vanilla = mix(vanillaSky, vanillaFog, h);
  return clamp(mix(c, vanilla, SKY_BIOME_TINT), vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));
}

#endif
