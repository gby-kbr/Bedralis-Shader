#ifndef GODRAYS_H
#define GODRAYS_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"
#include "origin/function/clouds.h"   // declares s_noisetexture: Sky fragment only

// Sun halo and light shafts painted on the sky.
//
// Without a depth buffer, shafts cannot be occluded by terrain or clouds, so they are
// a sky-only effect: a tight halo, a broad glow, and a ring of soft rays whose pattern
// comes from one read of the noise texture (strand channel) indexed by the angle
// around the sun.
//   col   : sky color (with cirrus) for this pixel
//   env   : environment unpacked from the Sky varyings
//   dir   : normalized view direction
//   phase : cirrus wind phase, reused for a very slow drift of the shafts
vec3 applySunGlow(vec3 col, environment env, vec3 dir, float phase) {
#if SUN_GLOW_ENABLED
  float sd = dot(dir, env.sunDir);
  if (sd < 0.35) {
    return col;
  }

  float sunUp = smoothstep(-0.12, 0.08, env.sunDir.y);
  float vis = sunUp * (1.0 - env.rainFactor);
  if (vis <= 0.0) {
    return col;
  }

  vec3 sunTint = mix(SUN_TINT_LOW, SUN_TINT_HIGH, smoothstep(0.0, 0.45, env.sunDir.y));

  float sd2 = sd * sd;
  float sd4 = sd2 * sd2;
  float sd8 = sd4 * sd4;
  float sd16 = sd8 * sd8;
  float halo = sd16 * sd16 * sd16 * SUN_HALO_STRENGTH + sd4 * sd2 * SUN_BROAD_GLOW;
  float glow = halo;

#if SUN_SHAFT_STRENGTH > 0.0
  // Angle around the sun, in the plane perpendicular to the sun direction.
  vec3 b1 = normalize(cross(env.sunDir, vec3(0.0, 0.0, 1.0)));
  vec3 b2 = cross(env.sunDir, b1);
  float ang = atan2(dot(dir, b2), dot(dir, b1));
  float u = ang * (SUN_SHAFT_COUNT / (2.0 * PI)) + phase;
  float ray = texture2DLod(s_noisetexture, vec2(u, 0.37), 0.0).r;
  ray = smoothstep(0.45, 0.95, ray);
  // Shafts reach further from the sun at dawn and dusk, and fade near the disc edge.
  float reach = sd8 * (1.0 + 0.6 * (env.dawnFactor + env.duskFactor));
  glow += ray * reach * SUN_SHAFT_STRENGTH;
#endif

  col += sunTint * glow * vis;
  return col;
#else
  return col;
#endif
}

#endif
