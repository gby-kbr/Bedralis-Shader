#ifndef INSTANCING
    $input v_worldPos
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
#include "origin/function/sky.h"

uniform vec4 TimeOfDay;
uniform vec4 FogColor;
uniform vec4 FogAndDistanceControl;
#endif

void main() {
#ifndef INSTANCING
    vec3 viewDir = -normalize(v_worldPos);
    viewDir.y = -viewDir.y;

    environment env;
    env.underwater = detectUnderwater(FogColor.rgb, FogAndDistanceControl.xy);
    env.rainFactor = detectRain(FogAndDistanceControl.xyz);
    env.dayFactor = 0.0;
    env.fogCol = FogColor.rgb;
    env = calculateSunParams(env, TimeOfDay.x);

    skycolor skycol = overworldSkyColors(env);

    gl_FragColor = vec4(renderOverworldSky(skycol, env, viewDir, true), 1.0);
#else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
