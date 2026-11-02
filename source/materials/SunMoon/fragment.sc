$input v_color0, v_texcoord0, v_worldPos

#include <bgfx_shader.sh>

SAMPLER2D_AUTOREG(s_SunMoonTexture);
uniform vec4 SunMoonColor;

void main() {
    vec4 tex = texture2D(s_SunMoonTexture, v_texcoord0);
    // v_color0.rgb is the sun/moon tint computed in the vertex shader.
    gl_FragColor = vec4(SunMoonColor.rgb * tex.rgb * v_color0.rgb, SunMoonColor.a * tex.a);
}
