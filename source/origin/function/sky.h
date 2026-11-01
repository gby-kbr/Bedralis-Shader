#ifndef SKY_H
#define SKY_H

#include "origin/configuration.h"
#include "origin/function/detection.h"

vec3 skyGradient(vec3 zenith, vec3 horizon, float y, float curve) {
    return mix(zenith, horizon, pow(1.0 - clamp(y, 0.0, 1.0), curve));
}

vec3 daySky(vec3 dir) {
    return skyGradient(SKY_DAY_ZENITH, SKY_DAY_HORIZON, dir.y, 3.0);
}

vec3 nightSky(vec3 dir) {
    return skyGradient(SKY_NIGHT_ZENITH, SKY_NIGHT_HORIZON, dir.y, 4.0);
}

vec3 dawnSky(vec3 dir, SkyEnvironment env) {
    vec3 zenith = mix(SKY_SUNSET_ZENITH, SKY_SUNRISE_ZENITH, env.sunrise);
    vec3 horizon = mix(SKY_SUNSET_HORIZON, SKY_SUNRISE_HORIZON, env.sunrise);
    float sunSide = pow(clamp(dot(dir, env.sunDir) * 0.5 + 0.5, 0.0, 1.0), 4.0);
    float band = pow(1.0 - clamp(dir.y, 0.0, 1.0), 2.5);
    vec3 base = skyGradient(zenith, horizon, dir.y, 2.0);
    vec3 glow = horizon * band * (0.25 + 0.9 * sunSide);
    return base * (0.8 + 0.2 * env.day) + glow * 0.8;
}

vec3 rainSky(vec3 dir, SkyEnvironment env) {
    float light = SKY_RAIN_NIGHT_LIGHT + (1.0 - SKY_RAIN_NIGHT_LIGHT) * clamp(env.day + 0.5 * env.twilight, 0.0, 1.0);
    return skyGradient(SKY_RAIN_ZENITH, SKY_RAIN_HORIZON, dir.y, 1.5) * light;
}

vec3 overworldSky(vec3 dir, SkyEnvironment env) {
    vec3 clear = mix(nightSky(dir), daySky(dir), env.day);
    clear = mix(clear, dawnSky(dir, env), env.twilight);
    return mix(clear, rainSky(dir, env), env.rain);
}

vec3 endSky(vec3 dir) {
    float h = 1.0 - clamp(abs(dir.y), 0.0, 1.0);
    vec3 col = mix(SKY_END_ZENITH, SKY_END_HORIZON, pow(h, 3.0));
    return col + SKY_END_GLOW * pow(h, 9.0) * 0.45;
}

vec3 renderSky(vec3 dir, SkyEnvironment env) {
    return mix(overworldSky(dir, env), endSky(dir), env.end);
}

#endif
