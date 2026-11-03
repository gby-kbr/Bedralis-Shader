#ifndef WEATHER_H
#define WEATHER_H

#include "origin/configuration.h"
#include "origin/function/utility.h"

// Rain and snow particles for the Weather material.
//
// The engine draws both with the same material. Rain is told apart from snow by its
// sprite sheet (rain has a tall sprite sheet) and its narrow width: the same test
// newb-aerotion uses. If it ever picks the wrong type the only effect is that the
// rain/snow tuning values swap, nothing breaks.

bool isRainParticle(vec4 uvOffsetAndScale, float particleWidth) {
  return (uvOffsetAndScale.w > 3.8 * uvOffsetAndScale.z) && particleWidth < 0.1;
}

// Particle size (width, length) after the Bedralis scale.
vec2 weatherParticleSize(bool isRain, vec2 engineSize) {
  if (isRain) {
    return engineSize * vec2(WEATHER_RAIN_WIDTH, WEATHER_RAIN_LENGTH);
  }
  return engineSize * WEATHER_SNOW_SIZE;
}

// Soft round speck, 0..1, from the quad UV (0..1). Used for snowflakes.
float weatherSpeck(vec2 quadUV) {
  vec2 g = 2.0 * quadUV - 1.0;
  g = 1.0 - g * g;
  float s = g.x * g.y;
  return s * s;
}

// Final color of one particle pixel.
//   diffuse : engine texture sample (rgb, a)
//   light   : engine lightmap color for this particle (already occlusion-aware)
//   skyFog  : atmospheric fog color (Phase 04) = sky horizon color
//   fogA    : engine fog intensity for the particle
//   isRain  : 1 for rain, 0 for snow
//   quadUV  : 0..1 position inside the particle quad
//   day     : daylight factor, 0..1
vec4 shadeWeather(vec4 diffuse, vec3 light, vec3 skyFog, float fogA, float isRain, vec2 quadUV, float day) {
  // Snow: blend the engine flake toward a soft round speck.
  float speck = weatherSpeck(quadUV);
  vec4 flake = vec4(diffuse.rgb, speck * max(diffuse.a, 0.6));
  vec4 snow = mix(diffuse, flake, WEATHER_SNOW_SOFT);
  snow.a *= WEATHER_SNOW_OPACITY;

  // Rain: take some of the sky color (misty), a little darker in daylight, thinner.
  vec4 rain = diffuse;
  rain.rgb *= mix(1.0, WEATHER_RAIN_DARKEN, day);
  rain.rgb = mix(rain.rgb, skyFog, WEATHER_RAIN_SKY_BLEND);
  rain.a *= WEATHER_RAIN_OPACITY;

  vec4 col = mix(snow, rain, isRain);
  col.rgb *= light;
  col.rgb = mix(col.rgb, skyFog, fogA);
  col.a *= 1.0 - fogA;
  return col;
}

#endif
