#ifndef WAVE_H
#define WAVE_H

#include "origin/configuration.h"

// Animated water surface, evaluated per pixel (normals) and optionally per vertex (height).
//
// Three layers, each fading out with distance at its own pace so far water stays
// calm instead of shimmering:
//   swell  : long gentle waves, visible far away (also drive the optional vertex waves)
//   medium : the main chop, bent by a slow domain warp so it does not look like a grid
//   detail : fine ripples, only close to the camera
// Plus small fast ripples while it rains.
//
// All wave vectors are integer multiples of 2*PI/64, so the pattern repeats every
// 64 blocks and positions can be wrapped before use. That keeps the sine arguments
// small far from the world origin (float precision) and keeps chunks continuous.

// Height of the long swell in blocks (at WATER_VERTEX_WAVE_SCALE = 1 the amplitude
// is at most 0.085 blocks). Used only when WATER_VERTEX_WAVES is enabled.
float waterWaveHeight(vec2 p, float t) {
  p = p - 64.0 * floor(p / 64.0);
  vec2 ks1 = vec2( 0.393, 0.295);
  vec2 ks2 = vec2(-0.295, 0.491);
  return (0.050 * sin(dot(p, ks1) + 0.9 * t) + 0.035 * sin(dot(p, ks2) - 0.7 * t)) * WATER_VERTEX_WAVE_SCALE;
}

//   p    : world position xz
//   t    : time in seconds
//   rain : 0..1, adds small fast ripples
//   dist : camera distance
vec3 waterNormal(vec2 p, float t, float rain, float dist) {
  p = p - 64.0 * floor(p / 64.0);

  // Swell.
  vec2 ks1 = vec2( 0.393, 0.295);
  vec2 ks2 = vec2(-0.295, 0.491);
  vec2 gSwell = ks1 * (0.050 * cos(dot(p, ks1) + 0.9 * t))
              + ks2 * (0.035 * cos(dot(p, ks2) - 0.7 * t));

  // Slow domain warp for the medium and detail layers.
  vec2 pw = p + 0.9 * vec2(sin(dot(p, vec2( 0.196, 0.098)) + 0.35 * t),
                           cos(dot(p, vec2(-0.098, 0.196)) - 0.28 * t));

  // Medium chop.
  vec2 km1 = vec2( 0.982,  0.687);
  vec2 km2 = vec2(-0.785,  1.277);
  vec2 km3 = vec2( 0.589, -1.963);
  vec2 gMed = km1 * (0.035 * cos(dot(pw, km1) + 1.0 * t))
            + km2 * (0.022 * cos(dot(pw, km2) - 1.3 * t))
            + km3 * (0.014 * cos(dot(pw, km3) + 1.7 * t));

  // Fine detail.
  vec2 kd1 = vec2( 2.258, -0.884);
  vec2 kd2 = vec2(-1.670,  1.866);
  vec2 gDet = kd1 * (0.0065 * cos(dot(pw, kd1) + 2.3 * t))
            + kd2 * (0.0055 * cos(dot(pw, kd2) - 2.0 * t));

  // Rain ripples.
  vec2 kr1 = vec2( 2.749, 0.884);
  vec2 kr2 = vec2(-1.178, 2.651);
  vec2 gRain = (kr1 * cos(dot(p, kr1) + 2.6 * t) + kr2 * cos(dot(p, kr2) - 2.2 * t)) * (0.1 * WATER_RAIN_RIPPLE * rain);

  float fadeSwell  = 1.0 - smoothstep(WATER_SWELL_FADE_START,  WATER_SWELL_FADE_END,  dist);
  float fadeMedium = 1.0 - smoothstep(WATER_WAVE_FADE_START,   WATER_WAVE_FADE_END,   dist);
  float fadeDetail = 1.0 - smoothstep(WATER_DETAIL_FADE_START, WATER_DETAIL_FADE_END, dist);

  vec2 g = gSwell * fadeSwell + (gMed + gRain) * fadeMedium + gDet * fadeDetail;
  g *= WATER_WAVE_STRENGTH;

  return normalize(vec3(-g.x, 1.0, -g.y));
}

// Shared wind field: x = gust strength (0..~2 in rain), y = flutter (-1..1).
vec2 windField(vec3 gPos, float t, float rain) {
  const float k = 0.09817477;
  vec3 p = gPos - 64.0 * floor(gPos / 64.0);
  float tt = t * PLANT_WAVE_SPEED;

  float gust = 0.5 + 0.28 * sin(dot(p.xz, vec2(2.0, 1.0) * k) - 0.55 * tt)
                   + 0.22 * sin(dot(p.xz, vec2(-1.0, 3.0) * k) - 0.83 * tt + 1.7);
  float wind = (0.35 + 0.65 * gust) * (1.0 + 1.0 * rain);

  float flutter = 0.65 * sin(dot(p, vec3(5.0, 3.0, 4.0) * k) + (2.4 + 1.6 * rain) * tt)
                + 0.35 * sin(dot(p, vec3(-3.0, 6.0, 2.0) * k) + (3.7 + 2.0 * rain) * tt + 1.3);
  return vec2(wind, flutter);
}

// Plant, leaf and vine sway, evaluated per vertex in the RenderChunk vertex shader.
//
// Wind is a slow travelling gust field plus fast per-position flutter. Every term is a
// function of the vertex position only, so vertices shared by neighbouring quads (leaf
// blocks) move together and no gaps open. All wave vectors are integer multiples of
// 2*PI/64, so positions can be wrapped to 64 blocks to keep float precision.
//
// What waves (same detection idea as Newb, tuned on vertex colors and block-local position):
//   leaves, tall grass, ferns, sugar cane : biome-tinted quads in the alpha-test pass
//   vines                                 : tinted quads sitting on a block wall
//   crops, flowers, saplings              : only the top vertices (top half of the texture tile)
// Ground plants are anchored at the bottom, so only top vertices move.
//
//   gPos     : absolute world position of the vertex
//   cPos     : chunk-local vertex position (a_position)
//   color    : vertex color (a_color0)
//   uv0      : decoded atlas uv
//   skyLight : lightmap sky value 0..1 (no wind in caves and under roofs)
//   t        : time in seconds
//   rain     : 0..1, stronger and faster wind
//   camDist  : camera distance, sway fades out at PLANT_WAVE_RANGE
vec3 plantWaveOffset(vec3 gPos, vec3 cPos, vec4 color, vec2 uv0, float skyLight, float t, float rain, float camDist) {
  if (camDist > PLANT_WAVE_RANGE) {
    return vec3(0.0, 0.0, 0.0);
  }

  vec3 bPos = fract(cPos);
  vec2 bPosC = abs(bPos.xz - 0.5);
  vec2 bPosH = fract(bPos.xz * 2.0);

  bool isTop = fract(uv0.y * PLANT_WAVE_ATLAS_ROWS) < 0.5;
  bool isColored = color.r != color.g || color.r != color.b;
  bool isLeaves = (isColored && (bPos.x + bPos.y + bPos.z < 0.001)) || color.a == 0.0;
  bool isGrass = isColored && color.r < 1.9 * color.g;
  bool isVines = (bPosC.x == 0.453125 && bPos.z == 0.0) || (bPosC.y == 0.453125 && bPos.x == 0.0);
  bool isFarmPlant = bPos.y == 0.9375 && (bPosC.x == 0.25 || bPosC.y == 0.25);
  bool isLeafLitter = bPos.y == 0.015625 && (bPosH.x + bPosH.y) == 0.0;
  bool isCross = bPosC.x > 0.32 && bPosC.x < 0.39 && bPosC.y > 0.32 && bPosC.y < 0.39;

  float weight = 0.0;
  float amp = 1.0;
  if (isColored && !isLeafLitter && (isLeaves || isGrass || isVines)) {
    if (isLeaves) {
      weight = 1.0;
      amp = PLANT_WAVE_LEAVES;
    } else if (isVines) {
      weight = isTop ? 0.5 : 1.0;
      amp = PLANT_WAVE_VINES;
    } else {
      weight = isTop ? 1.0 : 0.0;
    }
  } else if ((isFarmPlant || isCross) && isTop) {
    weight = 1.0;
  }

  if (weight == 0.0) {
    return vec3(0.0, 0.0, 0.0);
  }

  float fade = 1.0 - smoothstep(0.5 * PLANT_WAVE_RANGE, PLANT_WAVE_RANGE, camDist);

  vec2 w = windField(gPos, t, rain);
  float wind = w.x;
  float flutter = w.y;

  float sway = wind * (0.8 + 0.2 * flutter);
  float exposure = skyLight * skyLight;
  float a = PLANT_WAVE_STRENGTH * amp * weight * fade * exposure * sway;

  vec2 dir = vec2(0.8, 0.6);
  vec2 off = dir * a + vec2(-dir.y, dir.x) * (0.25 * a * flutter);

  return vec3(off.x, -0.5 * dot(off, off), off.y);
}

// Swinging lanterns and chains: a small pendulum about the top of their block, driven by
// the same wind field. Detection uses the exact block-local sizes of the lantern and chain
// models (as Newb does) plus a block light check so unrelated geometry is left alone.
// Hanging stacks pivot per block, so a long chain can open tiny gaps at its joints; keep
// LANTERN_WAVE small.
//
//   lightmapUV : x = block light, y = sky light (both 0..1)
vec3 lanternWaveOffset(vec3 gPos, vec3 cPos, vec2 uv0, vec2 lightmapUV, float t, float rain, float camDist) {
  if (camDist > PLANT_WAVE_RANGE || lightmapUV.x < 0.6) {
    return vec3(0.0, 0.0, 0.0);
  }

  vec3 bPos = fract(cPos);
  vec2 bPosC = abs(bPos.xz - 0.5);
  float texPosY = fract(uv0.y * PLANT_WAVE_ATLAS_ROWS);

  bool y6875 = bPos.y == 0.6875;
  bool y5625 = bPos.y == 0.5625;
  bool isLantern = ((y6875 || y5625) && bPosC.x == 0.125) || ((y5625 || bPos.y == 0.125) && bPosC.x == 0.1875);
  bool isChain = bPosC.x == 0.0625 && y6875;

  if (y5625 && (texPosY < 0.001 || (texPosY > 0.43 && texPosY < 0.93))) {
    isLantern = false;
  }

  if (!(isLantern || isChain)) {
    return vec3(0.0, 0.0, 0.0);
  }

  float fade = 1.0 - smoothstep(0.5 * PLANT_WAVE_RANGE, PLANT_WAVE_RANGE, camDist);
  vec2 w = windField(gPos, t, rain);
  float tt = t * PLANT_WAVE_SPEED;
  float phase = dot(floor(cPos), vec3(0.3927, 0.3927, 0.3927));
  float exposure = lightmapUV.y * lightmapUV.y;
  float amp = LANTERN_WAVE * w.x * fade * exposure;

  float along = amp * (0.6 + 0.4 * sin(1.9 * tt + phase));
  float across = 0.4 * amp * sin(1.3 * tt + phase + 1.0);

  vec2 dir = vec2(0.8, 0.6);
  vec3 axisAlong = vec3(-dir.y, 0.0, dir.x);
  vec3 axisAcross = vec3(dir.x, 0.0, dir.y);

  vec3 v = bPos - vec3(0.5, 1.0, 0.5);
  float c1 = cos(along);
  float s1 = sin(along);
  vec3 r = v * c1 + cross(axisAlong, v) * s1 + axisAlong * (dot(axisAlong, v) * (1.0 - c1));
  float c2 = cos(across);
  float s2 = sin(across);
  r = r * c2 + cross(axisAcross, r) * s2 + axisAcross * (dot(axisAcross, r) * (1.0 - c2));

  return r - v;
}

#endif
