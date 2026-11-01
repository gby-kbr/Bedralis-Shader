$input v_color0, v_texcoord0, v_worldPos

#include <bgfx_shader.sh>
#include "origin/function/sky.h"

SAMPLER2D_AUTOREG(s_SkyTexture);

void main() {
    vec3 dir = normalize(v_worldPos);
    vec3 stars = texture2D(s_SkyTexture, v_texcoord0).rgb;
    gl_FragColor = vec4(endSky(dir) + SKY_END_STARS * stars, 1.0);
}
