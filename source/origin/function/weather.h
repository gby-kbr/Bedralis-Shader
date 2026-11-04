#ifndef WEATHER_H
#define WEATHER_H

#include "origin/configuration.h"
#include "origin/function/clouds.h"
#include "origin/function/detection.h"
#include "origin/function/noise.h"
#include "origin/function/sky.h"
#include "origin/function/water.h"

float windblow(vec3 pos, float t){
  vec2 p = pos.xy/(1.0+pos.z);
  float val = sin(4.0*p.x + 2.0*p.y + 2.0*t + 3.0*p.y*p.x)*sin(p.y*2.0 + 0.2*t);
  val += sin(p.y - p.x + 0.2*t);
  return 0.25*val*val;
}

vec4 refl(
  inout vec4 color, skycolor skycol, environment env, vec3 viewDir, vec3 wPos, vec3 tiledCpos,
  vec3 CAMERA_POS, vec3 torchColor, vec2 lit, float camDist, float renderDist, highp float t
) {
  vec4 wetRefl = vec4(0.0,0.0,0.0,0.0);

  #ifndef GROUND_REFL
  if (env.rainFactor > 0.0) {
  #endif

    float wetness = lit.y*lit.y;

    float endDist = renderDist*0.6;
    if (camDist < endDist) {
      float cosR = max(viewDir.y, 0.0);
      float puddles = max(1.0 - GROUND_RAIN_PUDDLES*fastRand(tiledCpos.xz), 0.0);

      #ifndef GROUND_REFL
        wetness *= puddles;
        float reflective = wetness*env.rainFactor*GROUND_RAIN_WETNESS;
      #else
        float reflective = GROUND_REFL;
        if (!env.end && !env.nether) {
          reflective *= wetness;
        }

        wetness *= puddles;
        reflective = mix(reflective, wetness, env.rainFactor);
      #endif

      if (wPos.y < 0.0) {
        viewDir.y = -viewDir.y;
        wetRefl.rgb = renderSky(skycol, env, viewDir, t, false);

        #ifdef CLOUD_AURORA_REFLECTION
          vec4 cloudRefl = cloudAuroraReflection(skycol, env, viewDir, wPos, CAMERA_POS, t);
          wetRefl.rgb = mix(wetRefl.rgb, cloudRefl.rgb, cloudRefl.a);
        #endif

        wetRefl.rgb += torchColor*lit.x*TORCHLIGHT_INTENSITY;

        wetRefl.a = calculateFresnel(cosR, 0.03)*reflective;
        wetRefl.a *= clamp(2.0-2.0*camDist/endDist, 0.0, 1.0);
      }
    }

    color.rgb *= 1.0 - 0.4*wetness*env.rainFactor;

  #ifndef GROUND_REFL
  }
  #endif

  return wetRefl;
}

#endif
