#ifndef WAVE_H
#define WAVE_H

#include "origin/configuration.h"
#include "origin/function/utility.h"
#include "origin/function/noise.h"

#ifdef LANTERN_WAVE
void lanternWave(
  inout vec3 worldPos, vec3 cPos, vec3 bPos, vec2 bPosC, float texPosY, float rainFactor, vec2 uv1, float windStrength, highp float t
) {
  bool y6875 = bPos.y==0.6875;
  bool y5625 = bPos.y==0.5625;

  bool isLantern = ( (y6875 || y5625) && bPosC.x==0.125 ) || ( (y5625 || bPos.y==0.125) && (bPosC.x==0.1875) );
  bool isChain = bPosC.x==0.0625 && y6875;

  if (y5625 && (texPosY < 0.001 || (texPosY>0.43 && texPosY<0.93))) {
    isLantern = false;
  }

  if (uv1.x > 0.6 && (isChain || isLantern)) {
    float phase = dot(floor(cPos), vec3_splat(0.3927));
    vec2 theta = vec2(t + phase, t*1.4 + phase);
    theta = sin(vec2(theta.x,theta.x+0.7)) + rainFactor*sin(vec2(theta.y,theta.y+0.7));
    theta *= LANTERN_WAVE*windStrength;

    vec2 sinA = sin(theta);
    vec2 cosA = cos(theta);

    vec3 pivotPos = vec3(0.5,1.0,0.5) - bPos;
    worldPos.x += dot(pivotPos.xy, vec2(1.0-cosA.x, -sinA.x));
    worldPos.y += dot(pivotPos, vec3(sinA.x*cosA.y, 1.0-cosA.x*cosA.y, sinA.y));
    worldPos.z += dot(pivotPos, vec3(sinA.x*sinA.y, -cosA.x*sinA.y, 1.0-cosA.y));
  }
}
#endif

#ifdef EXTRA_PLANTS_WAVE
void extraPlantsFlag(inout bool shouldWave, vec2 uv0, bool isTop) {

  int texN = 64*int(uv0.y*32.0) + int(uv0.x*64.0);

  if (
    (texN>=18 && texN<=21) ||
    (texN>=180 && texN<=183) ||
    (texN>=189 && texN<=190) ||
    (texN>=449 && texN<=461) || (texN==687) ||
    (texN>=780 && texN<=781) ||
    (texN>=810 && texN<=811) ||
    (texN>=817 && texN<=818) ||
    (texN>=846 && texN<=849) || (texN>=852 && texN<=853) ||
    (texN>=940 && texN<=941) ||
    (texN>=1226 && texN<=1227)
  ) {
    shouldWave = true;
  } else if (
    (texN==6) ||
    (texN==8) ||
    (texN==25) ||
    (texN==85) ||
    (texN==111) ||
    (texN==148) ||
    (texN==195) ||
    (texN==226) ||
    (texN==339) ||
    (texN==391) ||
    (texN==394) ||
    (texN==400) ||

    (texN>=534 && texN<=535) ||
    (texN==568) ||
    (texN==649) ||
    (texN==688) ||
    (texN>=726 && texN<=728) ||
    (texN==764) ||
    (texN>=773 && texN<=775) ||
    (texN==786) ||
    (texN==788) ||
    (texN==822) ||
    (texN==838) ||
    (texN==881) ||
    (texN==883) ||
    (texN==937) ||
    (texN==938) ||
    (texN==943) ||
    (texN==973) ||
    (texN==1040) ||
    (texN>=1111 && texN<=1114) ||
    (texN==1116) ||
    (texN>=1121 && texN<=1123) ||
    (texN==1220)
  ) {
    shouldWave = isTop;
  } else if (
    (texN==23 || texN==551) ||
    (texN==617)
  ) {
    shouldWave = !isTop;
  }
}
#endif

void wave(
  inout vec3 worldPos, inout vec3 light, float rainFactor, vec2 uv1, vec2 lit,
  vec2 uv0, vec3 bPos, vec4 COLOR, vec3 cPos, vec3 tiledCpos, highp float t, sampler2D terrainTex,
  bool isColored, float camDist, bool isTreeLeaves
) {
  if (camDist > WAVE_RANGE) {
    return;
  }

  float waveFade = 2.0*max((camDist/WAVE_RANGE) - 0.5, 0.0);
  waveFade *= waveFade;

  float texPosY = fract(uv0.y*vec2(textureSize(terrainTex, 0)).y/16.0);

  vec2 bPosC = abs(bPos.xz-0.5);
  vec2 bPosH = fract(bPos.xz*2.0);

  bool isTop = texPosY < 0.5;
  bool isPlants = COLOR.r/COLOR.g<1.9;
  bool isVines = (bPosC.x==0.453125 && bPos.z==0.0) || (bPosC.y==0.453125 && bPos.x==0.0);
  bool isFarmPlant = (bPos.y==0.9375) && (bPosC.x==0.25 ||  bPosC.y==0.25);
  bool isRedStone = COLOR.r > 0.25 && COLOR.r > 3.0*COLOR.g  && COLOR.b == 0.0;
  bool isLeafLitter = bPos.y==0.015625 && (bPosH.x+bPosH.y)==0.0;
  bool shouldWave = ((isTreeLeaves || isPlants || isVines) && isColored && !isLeafLitter) || (isFarmPlant && isTop);

  float windStrength = lit.y*(noise1D(t*0.36) + rainFactor*0.4)*(1.0-waveFade);

  light *= isFarmPlant && !isTop ? 0.7 : 1.1;
  if (isColored && !isTreeLeaves && !isLeafLitter && uv0.y>0.214 && uv0.y<0.502 && !isRedStone) {
    light *= mix(isTop ? 1.2 : 1.2 - 1.2*(bPos.y>0.0 ? 1.5-bPos.y : 0.5), 1.0, waveFade);
  }

  #ifdef PLANTS_WAVE
    #ifdef EXTRA_PLANTS_WAVE
      extraPlantsFlag(shouldWave, uv0, isTop);
    #endif

    if (shouldWave) {
      float wave = PLANTS_WAVE*windStrength;

      if (isTreeLeaves) {
        wave *= 0.5;
      } else if (isVines) {
        wave *= fract(0.01+tiledCpos.y*0.5);
      } else if (isPlants && isColored && !isTop) {

        wave *= bPos.y > 0.0 ? bPos.y-1.0 : 0.0;
      }

      float phaseDiff = dot(cPos,vec3_splat(PI_QUART)) + fastRand(tiledCpos.xz + tiledCpos.y);
      wave *= 1.0 + mix(
        sin(t*WAVE_SPEED + phaseDiff),
        sin(t*WAVE_SPEED*1.5 + phaseDiff),
        rainFactor);

      worldPos.xyz -= vec3(wave, wave*wave*0.5, wave);
    }
  #endif

  #ifdef LANTERN_WAVE
    lanternWave(worldPos, cPos, bPos, bPosC, texPosY, rainFactor, uv1, windStrength, t);
  #endif
}

#endif
