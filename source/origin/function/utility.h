#ifndef UTILITY_H
#define UTILITY_H

#if !BGFX_SHADER_LANGUAGE_GLSL
vec2 mix(vec2 a, vec2 b, float t) { return lerp(a, b, t); }
vec3 mix(vec3 a, vec3 b, float t) { return lerp(a, b, t); }
vec4 mix(vec4 a, vec4 b, float t) { return lerp(a, b, t); }
#endif

float mixf(float a, float b, float t) { return a + (b - a) * t; }

#define PI 3.141592

mat2 rmat2(float t) {
    float sint = sin(t);
    float cost = cos(t);
    return mtxFromRows(vec2(cost, -sint), vec2(sint, cost));
}

float degToRad(float t) { return 0.0174533 * t; }

float luminance(vec3 x) { return dot(x, vec3(0.21, 0.71, 0.08)); }

#endif
