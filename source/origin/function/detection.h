#ifndef DETECTION_H
#define DETECTION_H

#include "origin/configuration.h"
#include "origin/function/utility.h"

struct environment {
  bool underwater;
  float rainFactor;
  float dayFactor;
  vec3 sunDir;
  vec3 moonDir;
  vec3 fogCol;
};

bool detectEnd(vec3 FOG_COLOR) {
  vec3 d = abs(FOG_COLOR*255.0 - vec3(64.0, 0.0, 64.0));
  bool exactMatch = d.x < 0.6 && d.y < 0.6 && d.z < 0.6;

  bool legacy = FOG_COLOR.r==FOG_COLOR.b && (FOG_COLOR.r-FOG_COLOR.g>0.24 || (FOG_COLOR.g==0.0 && FOG_COLOR.r>0.1));

  return exactMatch || legacy;
}

bool detectUnderwater(vec3 FOG_COLOR, vec2 FOG_CONTROL) {
  return !detectEnd(FOG_COLOR) && FOG_CONTROL.x==0.0 && FOG_CONTROL.y<0.8 && (FOG_COLOR.b>FOG_COLOR.r || FOG_COLOR.g>FOG_COLOR.r);
}

float detectRain(vec3 FOG_CONTROL) {
  vec2 clear = vec2(0.5 + 20.0/FOG_CONTROL.z, 1.0);
  vec2 rain = vec2(0.23, 0.70);
  vec2 factor = clamp((FOG_CONTROL.xy-clear)/(rain-clear), vec2(0.0,0.0), vec2(1.0,1.0));
  float val = factor.x*factor.y;
  return val*val*(3.0 - 2.0*val);
}

environment calculateSunParams(environment env, float TIME_OF_DAY) {
  float t = 2.0 * PI * TIME_OF_DAY;
  vec3 sunDir = vec3(sin(t), cos(t), 0.0); 
  vec3 moonDir = vec3(-sunDir.x, -sunDir.y, 0.0);
  env.dayFactor = sunDir.y;

  sunDir.yz = mul(rmat2(-degToRad(SUN_PATH_TILT)), sunDir.yz);
  sunDir.xz = mul(rmat2(degToRad(SUN_PATH_YAW)), sunDir.xz);

  moonDir.yz = mul(rmat2(-degToRad(MOON_PATH_TILT)), moonDir.yz);
  moonDir.xz = mul(rmat2(degToRad(MOON_PATH_YAW)), moonDir.xz);

  env.sunDir = sunDir;
  env.moonDir = moonDir;

  return env;
}

#endif
