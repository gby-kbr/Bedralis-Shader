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
    vec3 dir = normalize(v_worldPos);
    SkyEnvironment env = calculateEnvironment(TimeOfDay.x, FogColor.rgb, FogAndDistanceControl.xyz);
    gl_FragColor = vec4(renderSky(dir, env), 1.0);
#else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
