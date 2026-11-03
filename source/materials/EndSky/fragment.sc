$input v_color0, v_texcoord0, v_worldPos

#include <bgfx_shader.sh>
#include "origin/function/endsky.h"
#include "origin/function/tonemap.h"

void main() {
    gl_FragColor = vec4(colorCorrection(endSkyColor(normalize(v_worldPos))), 1.0);
}
