#ifndef UTILITY_H
#define UTILITY_H

#if !BGFX_SHADER_LANGUAGE_GLSL
vec2 mix(vec2 a, vec2 b, float t) { return lerp(a, b, t); }
vec3 mix(vec3 a, vec3 b, float t) { return lerp(a, b, t); }
vec4 mix(vec4 a, vec4 b, float t) { return lerp(a, b, t); }
#endif

#endif
