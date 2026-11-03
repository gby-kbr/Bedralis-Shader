#ifndef CLOUDS_H
#define CLOUDS_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"
#include "origin/function/sky.h"

// Cirrus clouds painted on the sky from a noise texture.
//
// Cost: three texture reads, a few multiplies, no loops and no procedural noise.
// Include this header only in fragment shaders of materials that have the buffer
// buffers/noisetexture.json: it declares the sampler (sampler name = s_ + buffer name).
//
// Texture channels (see scripts/generate_cirrus_texture.py), all uniform 0..1 so a
// threshold maps directly to a coverage fraction:
//   R : strands, G : veil mass, B : fine detail.
SAMPLER2D_AUTOREG(s_noisetexture);

// View direction to texture coordinates. A flattened dome projection (CIRRUS_DOME)
// stops the pattern from being squeezed to nothing near the horizon, and the rotation
// sets the direction of the strands and of the wind.
vec2 cirrusUV(vec3 dir) {
  vec2 p = dir.xz / (max(dir.y, 0.0) + CIRRUS_DOME);
  return rotate2(p, degToRad(CIRRUS_ROTATION)) * CIRRUS_SCALE;
}

// Cirrus thickness, 0 (clear) to 1 (thick), for a view direction.
//   phase : cirrusWindPhase(time), in [0, 4)
float cirrusThickness(vec3 dir, float phase) {
  vec2 uv = cirrusUV(dir);

  // Coarse layer: veil mass (G) and strands (R).
  vec3 n1 = texture2DLod(s_noisetexture, vec2(uv.x + phase, uv.y), 0.0).rgb;

  // Fine layer at twice the frequency, bent a little by the veil so the strands curve.
  // It drifts 1.5x faster than the coarse layer, so the clouds slowly change shape.
  float warp = (n1.g - 0.5) * CIRRUS_WARP;
  vec3 n2 = texture2DLod(s_noisetexture, vec2(uv.x * 2.0 + phase * 3.0 + warp * 0.8, uv.y * 2.0 + warp), 0.0).rgb;

  // Large-scale envelope: where the sky is clear and where the clouds gather.
  float low = texture2DLod(s_noisetexture, vec2(uv.x + phase, uv.y) * 0.5, 0.0).g;

  float mass = n1.g * 0.7 + n2.g * 0.3;
  float thr = 1.0 - CIRRUS_COVERAGE;
  float envelope = smoothstep(CIRRUS_ENV_LOW, CIRRUS_ENV_HIGH, low);
  float veil = smoothstep(thr - CIRRUS_SOFTNESS * 0.5, thr + CIRRUS_SOFTNESS * 0.5, mass) * envelope;

  float strands = max(smoothstep(CIRRUS_STRAND_LOW, CIRRUS_STRAND_HIGH, n1.r),
                      0.85 * smoothstep(CIRRUS_STRAND_LOW, CIRRUS_STRAND_HIGH, n2.r));
  float fine = (1.0 - CIRRUS_DETAIL) + CIRRUS_DETAIL * n2.b;

  float dens = veil * (CIRRUS_VEIL_BASE + (1.0 - CIRRUS_VEIL_BASE) * (0.35 * fine + 0.65 * strands));

  // Strands that stray out of the veil into clear sky.
  float wisps = strands * smoothstep(thr - CIRRUS_WISP_REACH, thr, mass)
                        * smoothstep(CIRRUS_ENV_LOW - 0.25, CIRRUS_ENV_HIGH - 0.10, low);

  return max(dens, wisps);
}

// Fraction of daylight reaching the clouds. High clouds stay lit a little after the
// sun has set, which is why this reaches further below the horizon than the sky does.
float cirrusLight(float sunElev) {
  return smoothstep(-0.16, 0.10, sunElev);
}

// Color of the lit side of the clouds.
vec3 cirrusLitColor(float sunElev) {
  vec3 sunTint = mix(SUN_TINT_LOW, SUN_TINT_HIGH, smoothstep(0.0, 0.45, sunElev));
  sunTint = mix(vec3(1.0, 1.0, 1.0), sunTint, CIRRUS_TWILIGHT_TINT);
  return mix(CIRRUS_COLOR_NIGHT, CIRRUS_COLOR_LIGHT * sunTint, cirrusLight(sunElev));
}

// Paints the cirrus over the sky color.
//   sky   : sky color for this pixel (also used to fade the clouds into the haze)
//   env   : environment unpacked from the Sky varyings
//   dir   : normalized view direction
//   phase : cirrus wind phase
vec3 applyCirrus(vec3 sky, environment env, vec3 dir, float phase) {
  float rainFade = 1.0 - smoothstep(0.0, 0.6, env.rainFactor);
  if (dir.y <= 0.03 || rainFade <= 0.0) {
    return sky;
  }

  float th = cirrusThickness(dir, phase);

  float elev = env.sunDir.y;
  float light = cirrusLight(elev);
  vec3 sunTint = mix(SUN_TINT_LOW, SUN_TINT_HIGH, smoothstep(0.0, 0.45, elev));
  sunTint = mix(vec3(1.0, 1.0, 1.0), sunTint, CIRRUS_TWILIGHT_TINT);

  vec3 lit = mix(CIRRUS_COLOR_NIGHT, CIRRUS_COLOR_LIGHT * sunTint, light);
  vec3 shade = mix(CIRRUS_COLOR_NIGHT * 0.8, CIRRUS_COLOR_SHADE * sunTint, light);
  vec3 col = mix(lit, shade, th * CIRRUS_SHADING);

  // Silver lining near the sun (thin parts glow most), stronger glow at dawn and dusk.
  float sd = clamp(dot(dir, env.sunDir), 0.0, 1.0);
  float sd2 = sd * sd;
  float silver = sd2 * sd2 * sd2 * CIRRUS_SUN_GLOW * light * (1.0 - th * 0.7);
  col += sunTint * silver;
  col *= 1.0 + 0.5 * (env.dawnFactor + env.duskFactor) * sd2 * sd;
  col = clamp(col, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));

  // Aerial perspective: clouds near the horizon fade into the haze.
  float t = 1.0 - clamp(dir.y, 0.0, 1.0);
  float t2 = t * t;
  col = mix(col, sky, 0.5 * t2 * t2);

  float alpha = th * CIRRUS_OPACITY * smoothstep(0.03, 0.30, dir.y) * rainFade * (0.35 + 0.65 * light);
  return mix(sky, col, alpha);
}

// Cirrus seen in the water: returns (color, alpha) for the reflected direction.
// Same texture and wind as the sky, so the reflection matches what is overhead.
//   time : seconds (ViewPositionAndTime.w)
vec4 cirrusReflection(vec3 dir, float time, float sunElev, float rain) {
  float rainFade = 1.0 - smoothstep(0.0, 0.6, rain);
  if (dir.y <= 0.03 || rainFade <= 0.0) {
    return vec4(0.0, 0.0, 0.0, 0.0);
  }
  float th = cirrusThickness(dir, cirrusWindPhase(time));
  float light = cirrusLight(sunElev);
  float alpha = th * CIRRUS_OPACITY * smoothstep(0.03, 0.30, dir.y) * rainFade * (0.35 + 0.65 * light);
  return vec4(cirrusLitColor(sunElev), alpha);
}

#endif
