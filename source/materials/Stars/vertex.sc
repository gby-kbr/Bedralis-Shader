$input a_color0, a_position, a_texcoord0
#ifdef INSTANCING
    $input i_data1, i_data2, i_data3
#endif

$output v_color0, v_texcoord0, v_worldPos

#include <bgfx_shader.sh>
#include "origin/function/celestial.h"

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

    environment env = calculateEnvironment(
        TimeOfDay.x,
        FogColor.rgb,
        vec3(FogAndDistanceControl.xy, RenderDistance.x),
        DimensionID.x);

    // Star visibility (night only, fades in rain) scales the star alpha.
    v_color0 = vec4(a_color0.rgb, a_color0.a * starVisibility(env));
    v_texcoord0 = a_texcoord0;
    v_worldPos = worldPosition;
    gl_Position = mul(u_viewProj, vec4(worldPosition, 1.0));
}
