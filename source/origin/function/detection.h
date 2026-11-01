#ifndef DETECTION_H
#define DETECTION_H

#include "origin/configuration.h"
#include "origin/function/utility.h"

struct SkyEnvironment {
    float end;
    float rain;
    float day;
    float night;
    float twilight;
    float sunrise;
    vec3 sunDir;
    vec3 moonDir;
};

float detectEnd(vec3 fogColor) {
    vec3 d = abs(fogColor * 255.0 - END_FOG_COLOR);
    return float(d.x < 0.6 && d.y < 0.6 && d.z < 0.6);
}

float detectRain(vec3 fogControl) {
    vec2 clear = vec2(0.5 + 20.0 / fogControl.z, 1.0);
    vec2 wet = vec2(0.23, 0.70);
    vec2 f = clamp((fogControl.xy - clear) / (wet - clear), vec2(0.0, 0.0), vec2(1.0, 1.0));
    float v = f.x * f.y;
    return v * v * (3.0 - 2.0 * v);
}

SkyEnvironment calculateEnvironment(float timeOfDay, vec3 fogColor, vec3 fogControl) {
    SkyEnvironment env;
    float a = 6.28318530718 * timeOfDay;
    float h = cos(a);
    env.end = detectEnd(fogColor);
    env.rain = detectRain(fogControl);
    env.day = smoothstep(-0.05, 0.35, h);
    env.night = 1.0 - smoothstep(-0.35, 0.05, h);
    env.twilight = exp(-h * h / 0.055);
    env.sunrise = step(0.5, timeOfDay);
    env.sunDir = vec3(-sin(a), h, 0.0);
    env.moonDir = -env.sunDir;
    return env;
}

#endif
