$input a_color0, a_position, a_texcoord0
#ifdef INSTANCING
    $input i_data1, i_data2, i_data3
#endif

$output v_color0, v_texcoord0, v_worldPos

#include <bgfx_shader.sh>
#include "origin/function/sky.h"

uniform vec4 SkyColor;
uniform vec4 FogColor;
uniform vec4 TimeOfDay;
uniform vec4 DimensionID;
uniform vec4 RenderDistance;
uniform vec4 FogAndDistanceControl;

void main() {
    vec3 worldPosition;
#ifdef INSTANCING
    mat4 model = mtxFromCols(i_data1, i_data2, i_data3, vec4(0.0, 0.0, 0.0, 1.0));
    worldPosition = instMul(model, vec4(a_position, 1.0)).xyz;
#else
    worldPosition = mul(u_model[0], vec4(a_position, 1.0)).xyz;
#endif

    // Sky colors are computed per vertex (the dome has few vertices), so the
    // fragment shader stays a plain pass-through. The dome is assumed to be
    // centered on the camera, so the world position is used as view direction.
    environment env = calculateEnvironment(
        TimeOfDay.x,
        FogColor.rgb,
        vec3(FogAndDistanceControl.xy, RenderDistance.x),
        DimensionID.x);

    vec3 skyCol = calculateSkyColor(
        env,
        a_color0.x,
        normalize(worldPosition),
        SkyColor.rgb,
        FogColor.rgb);

    v_color0 = vec4(skyCol, mix(SkyColor.a, FogColor.a, a_color0.x));
    v_texcoord0 = a_texcoord0;
    v_worldPos = worldPosition;
    gl_Position = mul(u_viewProj, vec4(worldPosition, 1.0));
}
