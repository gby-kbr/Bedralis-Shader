$input v_color0, v_texcoord0, v_worldPos

#include <bgfx_shader.sh>
#include "origin/function/endsky.h"
#include "origin/function/tonemap.h"

void main() {
    // v_color0 carries (cos, sin) of the sky rotation from the vertex shader.
    vec3 dir = endSkyRotate(normalize(v_worldPos), v_color0.xy);
    gl_FragColor = vec4(colorCorrection(endSkyColor(dir)), 1.0);
}
