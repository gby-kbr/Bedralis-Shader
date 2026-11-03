#ifndef ENDSKY_H
#define ENDSKY_H

#include "origin/configuration.h"

// Outer-space sky of The End, from a 360 degree equirectangular texture
// (assets/textures/environment/EndSkyTexture.png, made by scripts/generate_end_sky_texture.py).
// Cost: one texture read, one atan and one acos per pixel; no procedural noise.
// Include only in the EndSky fragment shader (it declares the sampler, buffer
// buffers/endskytexture.json).
SAMPLER2D_AUTOREG(s_endskytexture);

// Slowly turning sky: pass cos and sin of the angle from the vertex shader.
vec3 endSkyRotate(vec3 dir, vec2 cosSin) {
  return vec3(cosSin.x * dir.x - cosSin.y * dir.z, dir.y, cosSin.y * dir.x + cosSin.x * dir.z);
}

// Sky color for a view direction. u runs around the horizon, v from the zenith (0) down.
vec3 endSkyColor(vec3 dir) {
  vec2 uv = vec2(atan2(dir.x, dir.z) * 0.15915494 + 0.5, acos(clamp(dir.y, -1.0, 1.0)) * 0.31830989);
  return texture2DLod(s_endskytexture, uv, 0.0).rgb * END_SKY_BRIGHTNESS;
}

#endif
