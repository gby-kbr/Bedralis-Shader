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

  s.zenith = mix(s.zenith, mix(DAWN_ZENITH_COL, DAY_ZENITH_COL, env.rainFactor), dawnFactor);
  s.horizon = mix(s.horizon, mix(DAWN_HORIZON_COL, DAY_HORIZON_COL, env.rainFactor), dawnFactor);
  s.horizonEdge = mix(s.horizonEdge, mix(DAWN_EDGE_COL, DAY_EDGE_COL, env.rainFactor), dawnFactor);

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
  vec2 gh2 = g2;
  #ifdef SUN_GLOW_SHAPE

    vec3 sunR = cross(vec3(0.0, 1.0, 0.0), env.sunDir);
    sunR = length(sunR) > 0.001 ? normalize(sunR) : vec3(1.0, 0.0, 0.0);
    vec2 sunQ = abs(vec2(dot(viewDir, sunR), dot(viewDir, cross(env.sunDir, sunR)))) + 1e-4;
    sunQ /= max(sunQ.x, sunQ.y);
    vec2 sunP = pow(sunQ, vec2_splat(SUN_GLOW_SHAPE));
    float sunShape = pow(sunP.x + sunP.y, 1.0/SUN_GLOW_SHAPE)/length(sunQ);
    vec2 gs = g;
    gs.x *= sunShape*sunShape;
    vec2 gs1 = 1.0-mix(sqrt(gs), gs, env.rainFactor);
    gh2 = gs1*gs1;
  #endif
  vec2 g4 = gh2*gh2;
  vec2 g8 = g4*g4;
  float mg8 = (g8.x+g8.y)*mask*(1.0-0.9*env.rainFactor);

  float vh = 1.0 - viewDir.y*viewDir.y;
  float vh2 = vh*vh;
  vh2 = mixf(vh2, mixf(1.0, vh2*vh2, SKY_VOID_FACTOR), step(viewDir.y, 0.0));
  vh2 = mixf(vh2, 1.0, mg8);
  float vh4 = vh2*vh2;

  float rainMask = 1.0 - 0.5*env.rainFactor;
  float gradient1 = vh4*vh4;
  float gradient2 = 0.8*gradient1 + 0.2*vh2;
  gradient1 *= gradient1;
  gradient1 = mixf(gradient1*gradient1, 1.0, mg8)*rainMask;
  gradient2 = mixf(gradient2, 1.0, mg8)*rainMask;

  float dawnFactor = 1.0-env.dayFactor*env.dayFactor;
  

  float dw = dawnFactor*dawnFactor*dawnFactor;
  dw *= mixf(1.0, dw*dw, step(env.dayFactor, 0.0));

  float sw = clamp(DAWN_SPREAD*dw, 0.0, 1.0)*(1.0-env.rainFactor);
  float ew = clamp(DAWN_EDGE_SPREAD*dw, 0.0, 1.0)*(1.0-env.rainFactor);

  float sunFall = mixf(g2.x, pow(g1.x, DAWN_WIDTH), sw);
  float df = mixf(1.0, sunFall, dawnFactor*dawnFactor*(1.0-env.rainFactor));

  float dawnGradient1 = mixf(gradient1, pow(vh, DAWN_HEIGHT), sw);
  float dawnGradient2 = mixf(gradient2, vh, ew);

  vec3 sky = mix(skyCol.horizon, skyCol.horizonEdge, dawnGradient1*df*df);
  sky = mix(skyCol.zenith, sky, dawnGradient2*df);

  float nightMask = 1.0 - smoothstep(-0.12, 0.25, env.dayFactor);
  sky *= mixf(0.8+0.8*gradient2, 0.5+0.5*gradient2, nightMask);

  if (!isSkyPlane) {
    float source = max(0.0, (mg8-0.22)/0.78);
    source *= source;
    source *= source;
    sky *= 1.0 + 17.0*source*(1.0-env.rainFactor);
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

#endif
