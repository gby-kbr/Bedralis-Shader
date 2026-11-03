// The sky is drawn as a full-screen quad: the view direction is rebuilt per pixel
// from the inverse view-projection matrix, so the sky does not depend on the shape
// of the engine's sky mesh. (Same technique as the sky material of newb-aerotion.)
// The instanced variant draws nothing, otherwise the sky would be drawn twice.
#ifndef INSTANCING
  $input a_color0, a_position
  $output v_skyEnvA, v_worldPos, v_skyEnvB, v_skyEnvC
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include "origin/function/sky.h"

  uniform vec4 FogColor;
  uniform vec4 FogAndDistanceControl;
  uniform vec4 ViewPositionAndTime;
  uniform vec4 TimeOfDay;
  uniform vec4 DimensionID;
  uniform vec4 RenderDistance;
#endif

void main() {
#ifndef INSTANCING
    // Environment: all the trigonometry happens here, once per vertex of the quad.
    environment env = calculateEnvironment(
        TimeOfDay.x,
        FogColor.rgb,
        vec3(FogAndDistanceControl.xy, RenderDistance.x),
        DimensionID.x);

    v_skyEnvA = vec4(env.daylight, env.nightFactor, env.dawnFactor, env.duskFactor);
    v_skyEnvB = vec4(env.rainFactor, env.underwater ? 1.0 : 0.0, cirrusWindPhase(ViewPositionAndTime.w), env.dimension);
    v_skyEnvC = vec4(env.sunDir, 0.0);

    // Background quad.
    vec4 pos = vec4(a_position.xzy, 1.0);
    pos.xy = 2.0 * clamp(pos.xy, -0.5, 0.5);

    v_worldPos = mul(u_invViewProj, pos).xyz;
    gl_Position = pos;
#else
    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
