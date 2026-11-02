#ifndef INSTANCING
    $input a_position
    $output v_worldPos
#endif

#include <bgfx_shader.sh>

void main() {
#ifndef INSTANCING
    vec4 pos = vec4(a_position.xzy, 1.0);
    pos.xy = 2.0 * clamp(pos.xy, -0.5, 0.5);
    v_worldPos = mul(u_invViewProj, pos).xyz;
    gl_Position = pos;
#else
    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
