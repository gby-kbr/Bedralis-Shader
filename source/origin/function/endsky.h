#ifndef ENDSKY_H
#define ENDSKY_H

// The End sky: the 360 degree equirectangular texture end_sky.png
// (assets/textures/environment/end_sky.png, made by scripts/generate_end_sky_texture.py)
// shown as is. One texture read, one atan and one acos per pixel; no settings.
// Include only in the EndSky fragment shader (it declares the sampler, buffer
// buffers/endskytexture.json).
SAMPLER2D_AUTOREG(s_endskytexture);

// Sky color for a view direction. u runs around the horizon, v from the zenith (0) down.
vec3 endSkyColor(vec3 dir) {
  vec2 uv = vec2(atan2(dir.x, dir.z) * 0.15915494 + 0.5, acos(clamp(dir.y, -1.0, 1.0)) * 0.31830989);
  return texture2DLod(s_endskytexture, uv, 0.0).rgb;
}

#endif
