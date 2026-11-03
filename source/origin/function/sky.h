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

// 0 at the zenith, 1 at and below the horizon. Quadratic so the horizon band stays narrow.
float skyHorizonT(float viewDirY) {
  float t = 1.0 - clamp(viewDirY, 0.0, 1.0);
  return t * t;
}

// Wind phase for the cirrus, in [0, 4). It is periodic: the texture tiles every 1.0 and
// every layer offset is an integer multiple of the phase, so the wrap from 4 back to 0
// cannot be seen. Compute it in the vertex shader and pass it on.
float cirrusWindPhase(float time) {
  return fract(time * (CIRRUS_SPEED * 0.25)) * 4.0;
}

// The Sky vertex shader does the trigonometry once (4 vertices) and packs the result
// into three vec4 varyings:
//   A = (daylight, nightFactor, dawnFactor, duskFactor)
//   B = (rainFactor, underwater, cirrus wind phase, dimension)
//   C = (sunDir.xyz, unused)
// The fragment shader unpacks them with this function.
environment unpackSkyEnvironment(vec4 a, vec4 b, vec4 c, vec3 fogColor) {
  environment env;
  env.daylight = a.x;
  env.nightFactor = a.y;
  env.dawnFactor = a.z;
  env.duskFactor = a.w;
  env.rainFactor = b.x;
  env.underwater = b.y > 0.5;
  env.dimension = b.w;
  env.isNether = abs(b.w - float(DIM_NETHER)) < 0.5;
  env.isEnd = abs(b.w - float(DIM_END)) < 0.5;
  env.sunDir = c.xyz;
  env.dayFactor = c.y;
  env.moonDir = -c.xyz;
  env.fogCol = fogColor;
  return env;
}

#endif
