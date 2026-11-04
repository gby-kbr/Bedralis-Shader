#ifndef FOG_H
#define FOG_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"
#include "origin/function/sky.h"
#include "origin/function/water.h"

// Fog is computed in the vertex shaders and passed on as v_fog = (color, intensity).
// The color matches the sky horizon (Phase 02) so terrain fades into the sky,
// and the intensity adds height, horizon and weather fog on top of the engine's
// own distance fog.

float combineFog(float a, float b) {
  return 1.0 - (1.0 - a) * (1.0 - b);
}

// Fog color for a vertex.
//   viewDir : normalized direction from the camera to the vertex
vec3 atmosphericFogColor(environment env, vec3 viewDir, vec3 vanillaSky, vec3 vanillaFog) {
  // Underwater keeps the engine water fog (slightly desaturated); The End keeps its own fog.
  if (env.underwater) {
    return underwaterFogColor(vanillaFog);
  }
  if (env.isEnd) {
    return vanillaFog;
  }
  // Same function as the sky at the horizon (t = 1), so sky and fog match.
  return calculateSkyColor(env, 1.0, viewDir, vanillaSky, vanillaFog);
}

// Fog intensity for a vertex.
//   vanillaFog : the engine's own distance fog intensity (kept as the base)
//   distNorm   : camera distance divided by the fog max distance (as the engine does)
//   worldY     : world height of the vertex in blocks
//   viewDirY   : y component of the normalized camera-to-vertex direction
//   exposure   : 0 to 1, how much the vertex is open to the sky (sky light); 1 if unknown
float atmosphericFogIntensity(environment env, float vanillaFog, float distNorm, float worldY, float viewDirY, float exposure) {
  // Only the Overworld gets extra fog; underwater keeps the engine's value.
  if (env.underwater || env.isNether || env.isEnd) {
    return vanillaFog;
  }

  float d = clamp(distNorm, 0.0, 1.0);

  // Denser near the ground, thinning above HEIGHT_FOG_BASE.
  float heightAbove = max(worldY - HEIGHT_FOG_BASE, 0.0);
  float height = HEIGHT_FOG_STRENGTH * exp(-HEIGHT_FOG_FALLOFF * heightAbove) * d;

  // Denser when looking along the horizon, mostly far away.
  float alongHorizon = 1.0 - abs(viewDirY);
  float horizon = HORIZON_FOG_STRENGTH * alongHorizon*alongHorizon*alongHorizon * d*d;

  // Rain brings the fog closer.
  float weather = RAIN_FOG_STRENGTH * env.rainFactor * smoothstep(0.05, 1.0, d);

  float extra = combineFog(combineFog(height, horizon), weather) * clamp(exposure, 0.0, 1.0);
  return combineFog(vanillaFog, extra);
}

#endif
