#ifndef INSTANCING
    $input v_worldPos
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
#include "origin/function/sky.h"
#include "origin/function/tonemap.h"

uniform vec4 TimeOfDay;
uniform vec4 FogColor;
uniform vec4 FogAndDistanceControl;
#endif

void main() {
#ifndef INSTANCING
    vec3 viewDir = normalize(v_worldPos);

    environment env;
    env.end = false;
    env.nether = false;
    env.underwater = detectUnderwater(FogColor.rgb, FogAndDistanceControl.xy);
    env.rainFactor = detectRain(FogAndDistanceControl.xyz);
    env.dayFactor = 0.0;
    env.fogCol = FogColor.rgb;
    env = calculateSunParams(env, TimeOfDay.x);

    skycolor skycol = overworldSkyColors(env);

    vec3 skyColor = renderSky(skycol, env, -viewDir, 0.0, true);
    skyColor = colorCorrection(skyColor);

    gl_FragColor = vec4(skyColor, 1.0);
#else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
