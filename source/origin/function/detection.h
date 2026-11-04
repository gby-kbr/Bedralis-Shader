#ifndef DETECTION_H
#define DETECTION_H

#include "origin/configuration.h"
#include "origin/function/utility.h"

struct environment {
  bool end;
  bool nether;
  bool underwater;
  float rainFactor;
  float dayFactor;
  vec3 sunDir;
  vec3 moonDir;
  vec3 fogCol;
};

bool detectEnd(vec3 FOG_COLOR) {
  return FOG_COLOR.r==FOG_COLOR.b && (FOG_COLOR.r-FOG_COLOR.g>0.24 || (FOG_COLOR.g==0.0 && FOG_COLOR.r>0.1));
}

bool detectNether(vec3 FOG_COLOR, vec2 FOG_CONTROL) {

  float expectedFogX = 0.029 + (0.09*FOG_CONTROL.y*FOG_CONTROL.y);

  bool netherFogCtrl = (FOG_CONTROL.x<0.14  && abs(FOG_CONTROL.x-expectedFogX) < 0.02);
  bool netherFogCol = (FOG_COLOR.r+FOG_COLOR.g)>0.0;

  bool underLava = FOG_CONTROL.x == 0.0 && FOG_COLOR.b == 0.0 && FOG_COLOR.g < 0.18 && FOG_COLOR.r-FOG_COLOR.g > 0.1;

  return (netherFogCtrl && netherFogCol) || underLava;
}

bool detectUnderwater(vec3 FOG_COLOR, vec2 FOG_CONTROL) {
  return FOG_CONTROL.x==0.0 && FOG_CONTROL.y<0.8 && (FOG_COLOR.b>FOG_COLOR.r || FOG_COLOR.g>FOG_COLOR.r);
}

float detectRain(vec3 FOG_CONTROL) {

  vec2 clear = vec2(0.5 + 20.0/FOG_CONTROL.z, 1.0);
  vec2 rain = vec2(0.23, 0.70);
  vec2 factor = clamp((FOG_CONTROL.xy-clear)/(rain-clear), vec2(0.0,0.0), vec2(1.0,1.0));
  float val = factor.x*factor.y;
  return val*val*(3.0 - 2.0*val);
}

float detectDayFactor(vec3 FOG_COLOR) {
  return min(dot(FOG_COLOR, vec3(0.5,0.7,0.5)), 1.0);
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

environment detectEnvironment(float TIME_OF_DAY, vec3 FOG_COLOR, vec3 FOG_CONTROL) {
  environment env;
  env.end = detectEnd(FOG_COLOR);
  env.nether = detectNether(FOG_COLOR, FOG_CONTROL.xy);
  env.underwater = detectUnderwater(FOG_COLOR, FOG_CONTROL.xy);
  env.rainFactor = detectRain(FOG_CONTROL.xyz);
  env.fogCol = FOG_COLOR;
  env = calculateSunParams(env, TIME_OF_DAY);
  return env;
}

#endif
