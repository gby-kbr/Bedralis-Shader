#ifndef LIGHTING_H
#define LIGHTING_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"
#include "origin/function/celestial.h"

// Lighting works on top of the engine lightmap instead of replacing it, so the
// engine keeps handling brightness levels, night vision, the Nether and The End.
// Face shading (directional light) is already baked in by the engine: vertex
// colors for terrain and calculateLightIntensity() for actors.
// Everything below is computed per vertex; the fragment shader only mixes.

// Color of the light reaching sky-lit surfaces: time of day, moon phase and rain.
vec3 skyLightTint(environment env, float moonPhase) {
  // Underwater, Nether and End keep the engine lighting untouched.
  if (env.underwater || env.isNether || env.isEnd) {
    return vec3(1.0, 1.0, 1.0);
  }

  vec3 c = mix(LIGHT_NIGHT_TINT, LIGHT_DAY_TINT, env.daylight);
  c = mix(c, LIGHT_DAWN_TINT, env.dawnFactor);
  c = mix(c, LIGHT_DUSK_TINT, env.duskFactor);

  // New moon is darker than full moon (night only).
  float moonDim = LIGHT_NEWMOON_DIM * (1.0 - moonlightFromPhase(moonPhase));
  c *= 1.0 - moonDim * env.nightFactor;

  // Rain: flatter, grayer, slightly darker light.
  float luma = dot(c, vec3(0.299, 0.587, 0.114));
  c = mix(c, vec3(luma, luma, luma), LIGHT_RAIN_DESAT * env.rainFactor);
  c *= 1.0 - LIGHT_RAIN_DIM * env.rainFactor;
  return c;
}

// How strongly block light is tinted warm: weak in daylight, strong at night.
float blockLightWarmth(environment env) {
  if (env.underwater || env.isNether || env.isEnd) {
    return 0.0;
  }
  return mix(LIGHT_BLOCK_WARMTH_NIGHT, LIGHT_BLOCK_WARMTH_DAY, env.daylight);
}

// Applies the Bedralis light color to the engine lightmap color.
//   lightmapRGB : color sampled from the engine lightmap
//   lightUV     : (block light, sky light), 0 to 1, as used for the lightmap lookup
//   tint        : xyz = skyLightTint(), w = blockLightWarmth() (from the vertex shader)
vec3 applyBedralisLight(vec3 lightmapRGB, vec2 lightUV, vec4 tint) {
  float blockL = clamp(lightUV.x, 0.0, 1.0);
  float blockWeight = blockL * blockL * tint.w;
  vec3 t = mix(tint.xyz, LIGHT_BLOCK_TINT, blockWeight);
  return lightmapRGB * t;
}

#endif
