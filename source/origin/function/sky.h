#ifndef SKY_H
#define SKY_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/detection.h"

struct skycolor {
  vec3 zenith;
  vec3 horizon;
  vec3 horizonEdge;
};

vec3 spectrum(float x) {
  vec3 s = vec3(x-0.5, x, x+0.5);
  s = smoothstep(1.0,0.0,abs(s));
  return s*s;
}

skycolor overworldSkyColors(environment env) {
  skycolor s;
  float f = 1.0 + 2.0*(1.0-max(-env.dayFactor, 0.0));
  float nightFactor = step(env.dayFactor, 0.0);
  s.zenith = mix(DAY_ZENITH_COL, NIGHT_ZENITH_COL*f, nightFactor);
  s.horizon = mix(DAY_HORIZON_COL, NIGHT_HORIZON_COL*f, nightFactor);
  s.horizonEdge = mix(DAY_EDGE_COL, NIGHT_EDGE_COL*f, nightFactor);

  float dawnFactor = 1.0-env.dayFactor*env.dayFactor;
  dawnFactor *= dawnFactor*dawnFactor;
  dawnFactor *= mixf(1.0, dawnFactor*dawnFactor, nightFactor);
  s.zenith = mix(s.zenith, DAWN_ZENITH_COL, dawnFactor);
  s.horizon = mix(s.horizon, DAWN_HORIZON_COL, dawnFactor);
  s.horizonEdge = mix(s.horizonEdge, DAWN_EDGE_COL, dawnFactor);

  float zh = dot(s.zenith, vec3_splat(0.33));
  float hh = dot(s.horizon, vec3_splat(0.33));
  float rainMix = env.rainFactor*SKY_RAIN_MIX_FACTOR;
  s.zenith = mix(s.zenith, RAIN_ZENITH_COL*zh, rainMix);
  s.horizon = mix(s.horizon, RAIN_HORIZON_COL*hh, rainMix);
  s.horizonEdge = mix(s.horizonEdge, s.horizon, env.rainFactor);

  if (env.underwater) {
    vec3 underwaterFog = env.fogCol*env.fogCol*UNDERWATER_TINT;
    s.zenith = mix(2.0*underwaterFog, underwaterFog*zh, 0.8);
    s.horizon = mix(2.0*underwaterFog, underwaterFog*hh, 0.8);
    s.horizonEdge = s.horizon;
  }

  return s;
}

vec3 renderOverworldSky(skycolor skyCol, environment env, vec3 viewDir, bool isSkyPlane) {
  float avy = abs(viewDir.y);
  float mask = 0.5 + (0.5*viewDir.y/(0.4 + avy));

  vec2 g = clamp(0.5 - 0.5*vec2(dot(env.sunDir, viewDir), dot(env.moonDir, viewDir)), 0.0, 1.0);
  vec2 g1 = 1.0-mix(sqrt(g), g, env.rainFactor);
  vec2 g2 = g1*g1;
  vec2 g4 = g2*g2;
  vec2 g8 = g4*g4;
  float mg8 = (g8.x+g8.y)*mask*(1.0-0.9*env.rainFactor);

  float vh = 1.0 - viewDir.y*viewDir.y;
  float vh2 = vh*vh;
  vh2 = mixf(vh2, mixf(1.0, vh2*vh2, SKY_VOID_FACTOR), step(viewDir.y, 0.0));
  vh2 = mixf(vh2, 1.0, mg8);
  float vh4 = vh2*vh2;

  float gradient1 = vh4*vh4;
  float gradient2 = 0.8*gradient1 + 0.2*vh2;
  gradient1 *= gradient1;
  gradient1 = mixf(gradient1*gradient1, 1.0, mg8);
  gradient2 = mixf(gradient2, 1.0, mg8);

  float dawnFactor = 1.0-env.dayFactor*env.dayFactor;
  float df = mixf(1.0, g2.x, dawnFactor*dawnFactor);
  vec3 sky = mix(skyCol.horizon, skyCol.horizonEdge, gradient1*df*df);
  sky = mix(skyCol.zenith, sky, gradient2*df);

  sky *= 0.5+0.5*gradient2;
  sky *= (1.0 + (2.0*mg8 + 7.0*mg8*mg8)*mask)*mixf(1.0, mask, SKY_VOID_DARKNESS);

  if (!isSkyPlane) {
    float source = max(0.0, (mg8-0.22)/0.78);
    source *= source;
    source *= source;
    sky *= 1.0 + 15.0*source*(1.0-env.rainFactor);
  }

  #ifdef RAINBOW
    if (!env.underwater) {
      float rainbowFade = 0.5 + 0.5*viewDir.y;
      rainbowFade *= rainbowFade;
      rainbowFade *= mixf(RAINBOW_CLEAR, RAINBOW_RAIN, env.rainFactor);
      rainbowFade *= 0.5+0.5*env.dayFactor;
      sky += spectrum(24.2*(0.85-g.x))*rainbowFade*skyCol.horizon;
    }
  #endif

  return sky;
}

vec3 renderSky(skycolor skycol, environment env, vec3 viewDir, float t, bool isSkyPlane) {
  viewDir.y = -viewDir.y;
  return renderOverworldSky(skycol, env, viewDir, isSkyPlane);
}

#endif
