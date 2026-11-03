#ifndef WATER_H
#define WATER_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"
#include "origin/function/celestial.h"
#include "origin/function/wave.h"
#ifdef WATER_CLOUD_REFLECTION
  // Only the RenderChunk fragment shader defines this: it is the only place that
  // declares the noise sampler (clouds.h) for the water.
  #include "origin/function/clouds.h"
#endif

// Water shading for the RenderChunk fragment shader.
//
// Reflections are analytic and only cover what the shader can know without
// reading the scene: the sky gradient, the sun, the moon and the cirrus. There is no
// screen-space reflection (no usable depth/color buffer here, and a wrong
// reflection looks worse than none).

// More natural water color: slightly less saturated and nudged toward a soft teal,
// keeping the luminance of the lit engine color.
vec3 gradeWaterColor(vec3 c) {
  float luma = dot(c, vec3(0.299, 0.587, 0.114));
  c = mix(vec3(luma, luma, luma), c, WATER_SATURATION);
  vec3 tint = WATER_TINT * (luma / max(dot(WATER_TINT, vec3(0.299, 0.587, 0.114)), 0.001));
  return mix(c, tint, WATER_TINT_MIX);
}

// Fog color seen from underwater: the engine water fog, a little less saturated.
vec3 underwaterFogColor(vec3 vanillaFog) {
  float luma = dot(vanillaFog, vec3(0.299, 0.587, 0.114));
  return mix(vanillaFog, vec3(luma, luma, luma), WATER_UNDERWATER_DESAT);
}

// Shades one water pixel (surface seen from above).
//   water         : engine color after lighting; rgb = water color, a = opacity
//   worldPos      : world position of the pixel
//   camPos        : camera world position
//   time          : seconds (ViewPositionAndTime.w)
//   timeOfDay     : TimeOfDay.x
//   moonPhase     : MoonPhase.x
//   horizonColor  : sky horizon color (the atmospheric fog color)
//   rain          : 0..1 rain factor
//   skyExposure   : 0..1 sky light, hides reflections in caves and under roofs
vec4 shadeWater(vec4 water, vec3 worldPos, vec3 camPos, float time, float timeOfDay, float moonPhase, vec3 horizonColor, float rain, float skyExposure) {
  vec3 toPos = worldPos - camPos;
  float dist = length(toPos);
  vec3 V = toPos / max(dist, 0.0001);

  // Seen from below or edge-on: keep the engine look.
  if (V.y > -0.001) {
    return water;
  }

  vec3 N = waterNormal(worldPos.xz, time, rain, dist);

  // Fresnel: little reflection looking straight down, a lot at grazing angles.
  float cosT = clamp(dot(-V, N), 0.0, 1.0);
  float inv = 1.0 - cosT;
  float inv2 = inv * inv;
  float F = WATER_F0 + (1.0 - WATER_F0) * inv2 * inv2 * inv;

  vec3 R = reflect(V, N);
  R.y = max(R.y, 0.0);   // never reflect into the ground

  // Sky reflection: horizon color near the horizon, a darker zenith overhead.
  float h = 1.0 - clamp(R.y, 0.0, 1.0);
  h = h * h;
  vec3 zenith = horizonColor * mix(WATER_SKY_ZENITH_RATIO, vec3(0.9, 0.9, 0.9), rain);
  vec3 sky = mix(zenith, horizonColor, h);
#ifdef WATER_CLOUD_REFLECTION
  // Cirrus in the reflection (same texture and wind as the sky).
  vec4 cloudRefl = cirrusReflection(R, time, sunDirection(timeOfDay).y, rain);
  sky = mix(sky, cloudRefl.rgb, cloudRefl.a);
#endif

  // Sun and moon glints (also a faint broad glow around the sun).
  vec3 sunDir = sunDirection(timeOfDay);
  float sunUp = smoothstep(0.0, 0.12, sunDir.y);
  float sd = max(dot(R, sunDir), 0.0);
  vec3 sunColor = mix(SUN_TINT_LOW, SUN_TINT_HIGH, smoothstep(0.0, 0.45, sunDir.y));
  vec3 glint = sunColor * (pow(sd, 600.0) * WATER_SUN_STRENGTH + pow(sd, 24.0) * 0.12) * sunUp;

  vec3 moonDir = moonDirection(timeOfDay);
  float moonUp = smoothstep(0.0, 0.12, moonDir.y);
  float md = max(dot(R, moonDir), 0.0);
  glint += MOON_TINT * pow(md, 600.0) * WATER_MOON_STRENGTH * moonlightFromPhase(moonPhase) * moonUp;

  // Glints fade in rain and with low sky exposure, and are stronger at grazing angles.
  glint *= (1.0 - 0.9 * rain) * skyExposure * clamp(F * 4.0, 0.0, 1.0);

  vec3 base = gradeWaterColor(water.rgb);
  float refl = clamp(F * WATER_REFLECT_STRENGTH, 0.0, 1.0) * skyExposure;
  vec3 col = clamp(mix(base, sky, refl) + glint, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));

  // Clearer when looking down, more opaque at grazing angles.
  float a = clamp(water.a * WATER_ALPHA_SCALE + F * WATER_FRESNEL_ALPHA * skyExposure, 0.0, 1.0);
  return vec4(col, a);
}

#endif
