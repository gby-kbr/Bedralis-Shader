#ifndef CLOUDS_H
#define CLOUDS_H

#include "origin/configuration.h"
#include "origin/function/detection.h"
#include "origin/function/noise.h"
#include "origin/function/sky.h"

float cloudNoise2D(vec2 p, highp float t, float rain) {
  t *= CLOUD1_SPEED;
  p += t;
  p.y += 3.0*sin(0.3*p.x + 0.1*t);

  vec2 p0 = floor(p);
  vec2 u = p-p0;
  u *= u*(3.0-2.0*u);
  vec2 v = 1.0-u;

  float n = mix(
    mix(rand(p0),rand(p0+vec2(1.0,0.0)), u.x),
    mix(rand(p0+vec2(0.0,1.0)),rand(p0+vec2(1.0,1.0)), u.x),
    u.y
  );
  n *= 0.5 + 0.5*sin(p.x*0.6 - 0.5*t)*sin(p.y*0.6 + 0.8*t);
  n = min(n*(1.0+rain), 1.0);
  return n*n;
}

vec4 renderCloudsSimple(skycolor skycol, vec3 pos, highp float t, float rain) {
  pos.xz *= CLOUD1_SCALE;
  float d = cloudNoise2D(pos.xz, t, rain);
  vec4 col = vec4(skycol.horizonEdge + skycol.zenith, smoothstep(0.1,0.6,d));
  col.rgb += 1.5*dot(col.rgb, vec3(0.3,0.4,0.3))*smoothstep(0.6,0.2,d)*col.a;
  col.rgb *= 1.0 - 0.8*rain;
  return col;
}

float cloudDf(vec3 pos, float rain, vec2 boxiness) {
  boxiness *= 0.999;
  vec2 p0 = floor(pos.xz);
  vec2 u = max((pos.xz-p0-boxiness.x)/(1.0-boxiness.x), 0.0);
  u *= u*(3.0 - 2.0*u);

  vec4 r = vec4(rand(p0), rand(p0+vec2(1.0,0.0)), rand(p0+vec2(1.0,1.0)), rand(p0+vec2(0.0,1.0)));
  r = smoothstep(0.1001+0.2*rain, 0.1+0.2*rain*rain, r);

  float n = mix(mix(r.x,r.y,u.x), mix(r.w,r.z,u.x), u.y);

  n *= 1.0 - 1.5*smoothstep(boxiness.y, 2.0 - boxiness.y, 2.0*abs(pos.y-0.5));

  n = max(1.25*(n-0.2), 0.0);
  n *= n*(3.0 - 2.0*n);
  return n;
}

vec4 renderCloudsRounded(
    vec3 vDir, vec3 vPos, float rain, float time, vec3 horizonCol, vec3 zenithCol,
    const int steps, const float thickness, const float thickness_rain, const float speed,
    const vec2 scale, const float density, const vec2 boxiness
) {
  float height = 7.0*mix(thickness, thickness_rain, rain);
  float stepsf = float(steps);

  vec3 deltaP;
  deltaP.y = 1.0;
  deltaP.xz = height*scale*vDir.xz/(0.02+0.98*abs(vDir.y));

  vec3 pos;
  pos.y = 0.0;
  pos.xz = scale*(vPos.xz + vec2(1.0,0.5)*(time*speed));
  pos += deltaP;

  deltaP /= -stepsf;

  vec2 d = vec2(0.0,1.0);
  for (int i=1; i<=steps; i++) {
    float m = cloudDf(pos, rain, boxiness);
    d.x += m;
    d.y = mix(d.y, pos.y, m);
    pos += deltaP;
  }
  d.x *= smoothstep(0.03, 0.1, d.x);
  d.x /= (stepsf/density) + d.x;

  if (vPos.y < 0.0) {
    d.y = 1.0 - d.y;
  }

  vec4 col = vec4(zenithCol + horizonCol, d.x);
  col.rgb += dot(col.rgb, vec3(0.3,0.4,0.3))*d.y*d.y;
  col.rgb *= 1.0 - 0.8*rain;
  return col;
}

float cloudsNoiseVr(vec2 p, float t) {
  float n = fastVoronoi2(p + t, 1.8);
  n *= fastVoronoi2(3.0*p + t, 1.5);
  n *= fastVoronoi2(9.0*p + t, 0.4);
  n *= fastVoronoi2(27.0*p + t, 0.1);

  return n*n;
}

vec4 renderClouds(vec2 p, float t, float rain, vec3 horizonCol, vec3 zenithCol, const vec2 scale, const float velocity, const float shadow) {
  p *= scale;
  t *= velocity;

  float a = cloudsNoiseVr(p, t);
  float b = cloudsNoiseVr(p + CLOUD3_SHADOW_OFFSET*scale, t);

  p = 1.4 * p.yx + vec2(7.8, 9.2);
  t *= 0.5;
  float c = cloudsNoiseVr(p, t);
  float d = cloudsNoiseVr(p + CLOUD3_SHADOW_OFFSET*scale, t);

  vec2 tr = vec2(0.6, 0.7) - 0.12*rain;
  a = smoothstep(tr.x, tr.y, a);
  c = smoothstep(tr.x, tr.y, c);

  b *= smoothstep(0.2, 0.8, b);
  d *= smoothstep(0.2, 0.8, d);

  vec4 col;
  col.a = a + c*(1.0-a);
  col.rgb = horizonCol + horizonCol.ggg;
  col.rgb = mix(col.rgb, 0.5*(zenithCol + zenithCol.ggg), shadow*mix(b, d, c));
  col.rgb *= 1.0-0.7*rain;

  return col;
}

#ifdef AURORA
vec4 renderAurora(vec3 p, float t, float rain, float dayFactor) {
  t *= AURORA_VELOCITY;
  p.xz *= AURORA_SCALE;
  p.xz += 0.05*sin(p.x*4.0 + 20.0*t);

  float d0 = sin(p.x*0.1 + t + sin(p.z*0.2));
  float d1 = sin(p.z*0.1 - t + sin(p.x*0.2));
  float d2 = sin(p.z*0.1 + 1.0*sin(d0 + d1*2.0) + d1*2.0 + d0*1.0);
  d0 *= d0; d1 *= d1; d2 *= d2;
  d2 = d0/(1.0 + d2/AURORA_WIDTH);

  float mask = max(-dayFactor, 0.0);
  mask *= mask*(1.0-0.8*rain);
  return vec4(AURORA*mix(AURORA_COL1,AURORA_COL2,d1),1.0)*d2*mask;
}
#endif

vec4 cloudAuroraReflection(skycolor skycol, environment env, vec3 viewDir, vec3 wPos, vec3 CAMERA_POS, highp float t) {
  vec2 cloudPos = wPos.xz;
  cloudPos += (187.0-(wPos.y+CAMERA_POS.y))*viewDir.xz/viewDir.y;
  float fade = clamp(2.0 - 0.005*length(cloudPos), 0.0, 1.0);
  cloudPos += CAMERA_POS.xz;

  vec4 refl = vec4_splat(0.0);

  #ifdef AURORA
    vec4 aurora = renderAurora(cloudPos.xyy, t, env.rainFactor, smoothstep(0.2, -0.2, env.dayFactor));
    aurora.a *= fade;
    refl = vec4(2.0*aurora.rgb*aurora.a, aurora.a);
  #endif

  #if CLOUD_TYPE == 1
    vec4 clouds = renderCloudsSimple(skycol, cloudPos.xyy, t, env.rainFactor);
    clouds.a *= fade;
    refl = vec4(mix(refl.rgb, clouds.rgb, clouds.a), min(refl.a + clouds.a, 1.0));
  #endif

  return refl;
}

#endif
