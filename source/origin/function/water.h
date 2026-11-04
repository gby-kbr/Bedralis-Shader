#ifndef WATER_H
#define WATER_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"
#include "origin/function/sky.h"
#include "origin/function/clouds.h"
#include "origin/function/noise.h"

float calculateFresnel(float cosR, float r0) {
  float a = 1.0-cosR;
  float a2 = a*a;
  return r0 + (1.0-r0)*a2*a2*a;
}

vec4 water(
  inout vec4 color, inout vec3 wPos, skycolor skycol, environment env, vec4 COLOR, vec3 viewDir,
  vec3 cPos, vec3 tiledCpos, vec3 gPos, vec3 CAMERA_POS, vec3 light, vec3 torchColor, vec2 lit,
  float fractCposY, float camDist, highp float t
) {
  vec2 bump = vec2_splat(movingNoise2D(gPos.xz + gPos.yy, WATER_WAVE_SPEED*t, 0.6));

  vec3 nrm;
  if (fractCposY > 0.0) {
    nrm.xz = bump*WATER_BUMP;
    nrm.y = -1.0;

  } else {
    bump *= 0.5 + 0.5*sin(3.0*t*WATER_WAVE_SPEED + cPos.y*PI_HALF);
    nrm.xz = normalize(viewDir.xz) + bump.y*(1.0-viewDir.xz*viewDir.xz)*WATER_BUMP;
    nrm.y = bump.x*WATER_BUMP;
  }
  nrm = normalize(nrm);

  float cosR = dot(nrm, viewDir);
  viewDir = viewDir - 2.0*cosR*nrm ;

  vec3 waterRefl = renderSky(skycol, env, viewDir, t, false);

  #if defined(CLOUD_AURORA_REFLECTION)
    if (viewDir.y < 0.0) {
      vec4 cloudRefl = cloudAuroraReflection(skycol, env, viewDir, wPos, CAMERA_POS, t);
      waterRefl = mix(waterRefl, cloudRefl.rgb, cloudRefl.a);
    }
  #endif

  float tc = 0.5+0.5*sin(16.0*viewDir.x)*sin(16.0*viewDir.z);
  waterRefl += torchColor*TORCHLIGHT_INTENSITY*lit.x*tc*tc;

  if (!env.end) {
    waterRefl *= 0.05 + lit.y*1.14;
  }

  #ifdef WATER_REFL_MASK
    float mask = 0.05+0.05*sin(viewDir.x*12.0)*sin(viewDir.z*6.0);
    waterRefl *= smoothstep(mask-0.2,mask+0.13,viewDir.y*viewDir.y);
  #endif

  cosR = abs(cosR);
  float fresnel = calculateFresnel(cosR, 0.07);
  float opacity = 1.0-cosR;

  color.rgb *= 0.22*WATER_TINT*(1.0-0.8*fresnel);
  color.a = mix(COLOR.a*WATER_TRANSPARENCY, 1.0, opacity*opacity);

  #ifdef WATER_WAVE
    if (camDist < 14.0) {
      wPos.y -= 0.5*(bump.x+0.5)*WATER_BUMP;
    }
  #endif

  return vec4(waterRefl, fresnel);
}

#endif
