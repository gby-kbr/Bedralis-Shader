#ifndef INSTANCING
  $input v_skyEnvA, v_worldPos, v_skyEnvB, v_skyEnvC
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include "origin/function/clouds.h"
  #include "origin/function/godrays.h"
  #include "origin/function/tonemap.h"

  uniform vec4 SkyColor;
  uniform vec4 FogColor;
#endif

void main() {
#ifndef INSTANCING
    vec3 viewDir = normalize(v_worldPos);
    environment env = unpackSkyEnvironment(v_skyEnvA, v_skyEnvB, v_skyEnvC, FogColor.rgb);

    vec3 color = calculateSkyColor(env, skyHorizonT(viewDir.y), viewDir, SkyColor.rgb, FogColor.rgb);

  #if CIRRUS_ENABLED
    if (!env.underwater && !env.isNether && !env.isEnd) {
        color = applyCirrus(color, env, viewDir, v_skyEnvB.z);
    }
  #endif

  #if SUN_GLOW_ENABLED
    if (!env.underwater && !env.isNether && !env.isEnd) {
        color = applySunGlow(color, env, viewDir, v_skyEnvB.z);
    }
  #endif

    gl_FragColor = vec4(colorCorrection(color), 1.0);
#else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
