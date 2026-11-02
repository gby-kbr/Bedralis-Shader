#ifndef DETECTION_H
#define DETECTION_H

#include "origin/configuration.h"
#include "origin/function/utility.h"

// Centralized environment state. Fill it once per shader invocation with
// calculateEnvironment() and pass it to the sky, fog, lighting, water, cloud
// and weather functions instead of recomputing the same values in each.
struct environment {
  bool underwater;
  float rainFactor;     // 0 = clear, 1 = full rain (smooth)
  float dayFactor;      // sine of sun elevation, -1..1 (before orbit tilt/yaw)
  float daylight;       // 0 = night, 1 = full day (smooth)
  float nightFactor;    // 0 = day, 1 = full night (smooth)
  float dawnFactor;     // twilight weight while the sun is rising (smooth)
  float duskFactor;     // twilight weight while the sun is setting (smooth)
  float dimension;      // raw dimension ID, compare with DIM_* macros
  bool isNether;
  bool isEnd;
  vec3 sunDir;
  vec3 moonDir;
  vec3 fogCol;
};

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

// Dimension from the DimensionID uniform value (pass DimensionID.x).
// Rounds first so a slightly off float still maps to the right dimension.
float detectDimension(float dimensionId) {
  return floor(dimensionId + 0.5);
}

// Direction of a body on its orbit. angle = 2*PI*TimeOfDay (+PI for the moon).
// Shared by calculateSunParams() and by shaders that only need the direction.
vec3 orbitDirection(float angle, float tilt, float yaw) {
  vec3 d = vec3(sin(angle), cos(angle), 0.0);
  d.yz = rotate2(d.yz, -degToRad(tilt));
  d.xz = rotate2(d.xz, degToRad(yaw));
  return d;
}

vec3 sunDirection(float timeOfDay) {
  return orbitDirection(2.0 * PI * timeOfDay, SUN_PATH_TILT, SUN_PATH_YAW);
}

vec3 moonDirection(float timeOfDay) {
  return orbitDirection(2.0 * PI * timeOfDay + PI, MOON_PATH_TILT, MOON_PATH_YAW);
}

environment calculateSunParams(environment env, float TIME_OF_DAY) {
  float t = 2.0 * PI * TIME_OF_DAY;
  env.dayFactor = cos(t);   // sun elevation before orbit tilt/yaw

  env.sunDir = sunDirection(TIME_OF_DAY);
  env.moonDir = moonDirection(TIME_OF_DAY);

  // Smooth day/night/twilight weights derived from the sun elevation.
  // The sun is rising while elevation increases, which is when sin(t) < 0.
  float elevation = env.dayFactor;
  env.daylight = smoothstep(-0.10, 0.25, elevation);
  env.nightFactor = 1.0 - smoothstep(-0.25, 0.05, elevation);
  float twilight = clamp(1.0 - abs(elevation) / 0.30, 0.0, 1.0);
  twilight = twilight*twilight*(3.0 - 2.0*twilight);
  // (smoothstep needs edge0 < edge1 in GLSL, so the reversed ramp is written as 1 - smoothstep.)
  float rising = 1.0 - smoothstep(-0.05, 0.05, sin(t));
  env.dawnFactor = twilight * rising;
  env.duskFactor = twilight * (1.0 - rising);

  return env;
}

// Builds the full environment state in one place.
//   timeOfDay    : TimeOfDay.x
//   fogColor     : FogColor.rgb
//   fogControl   : vec3(FogControl.xy, RenderDistance)
//   dimensionId  : DimensionID.x
environment calculateEnvironment(float timeOfDay, vec3 fogColor, vec3 fogControl, float dimensionId) {
  environment env;
  env.fogCol = fogColor;
  env.underwater = detectUnderwater(fogColor, fogControl.xy);
  env.rainFactor = detectRain(fogControl);

  env.dimension = detectDimension(dimensionId);
  env.isNether = env.dimension == float(DIM_NETHER);
  env.isEnd = env.dimension == float(DIM_END);

  env = calculateSunParams(env, timeOfDay);
  return env;
}

#endif
