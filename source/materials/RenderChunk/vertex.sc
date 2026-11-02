$input a_color0, a_position, a_texcoord0, a_texcoord1
#ifdef INSTANCING
    $input i_data1, i_data2, i_data3
#endif

$output v_clipPosition, v_color0, v_ditheringAndMaskTinting, v_fog, v_lightmapUV, v_lightTint, v_texcoord0, v_worldPos, v_worldPosition

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/TAAUtil.dragonh>
#include "origin/function/fog.h"
#include "origin/function/lighting.h"

uniform vec4 RenderChunkFogAlpha;
uniform vec4 FogAndDistanceControl;
uniform vec4 ViewPositionAndTime;
uniform vec4 FogColor;
uniform vec4 MeshContext;
uniform vec4 SkyColor;
uniform vec4 TimeOfDay;
uniform vec4 DimensionID;
uniform vec4 RenderDistance;
uniform vec4 MoonPhase;

void main() {
#ifdef INSTANCING
    mat4 model = mtxFromCols(i_data1, i_data2, i_data3, vec4(0.0, 0.0, 0.0, 1.0));
    vec3 worldPos = instMul(model, vec4(a_position, 1.0)).xyz;
#else
    vec3 worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
#endif

    vec4 color;
#ifdef RENDER_AS_BILLBOARDS
    worldPos += vec3_splat(0.5);
    vec3 viewDir = normalize(worldPos - ViewPositionAndTime.xyz);
    vec3 boardPlane = normalize(cross(vec3(0.0, 1.0, 0.0), viewDir));
    worldPos -= (cross(viewDir, boardPlane) * (a_color0.z - 0.5)) + (boardPlane * (a_color0.x - 0.5));
    color = vec4_splat(1.0);
#else
    color = a_color0;
#endif

#if WATER_VERTEX_WAVES && defined(TRANSPARENT)
    // Experimental (off by default): lift or lower the top water vertices with the swell.
    // The offset only depends on world xz and time, so neighboring water vertices move
    // together; fract(y) > 0.5 limits it to surface vertices.
    if (a_color0.a < 0.95 && fract(worldPos.y) > 0.5 && !detectUnderwater(FogColor.rgb, FogAndDistanceControl.xy)) {
        worldPos.y += waterWaveHeight(worldPos.xz, ViewPositionAndTime.w);
    }
#endif

    vec4 fogAndDist = FogAndDistanceControl;
    if (MeshContext.x > 0.5) {
        fogAndDist = vec4(0.99, 1.0, 100000.0, 100000.0);
    }

    uvec2 uv0_16 = uvec2(round(a_texcoord0 * 65535.0));
    vec2 uv0 = vec2(float((uv0_16.x & 32767u) << uint(1)), float((uv0_16.y & 32767u) << uint(1))) * 1.525902189314365386962890625e-05;
    uv0.x += 3.0517578125e-05 * ((2.0 * float((uv0_16.x & 32768u) >> uint(15))) - 1.0);
    uv0.y += 3.0517578125e-05 * ((2.0 * float((uv0_16.y & 32768u) >> uint(15))) - 1.0);

    uvec2 uv1_16 = uvec2(round(a_texcoord1 * 65535.0));
    float lightmapX = float((uv1_16.y >> 4u) & 15u) * 0.06666667;
    float lightmapY = float(uv1_16.y & 15u) * 0.06666667;
    vec2 lightmapUV = vec2(lightmapX, lightmapY);

    float ditherX = float((uv1_16.x & 256u) != 0u ? 1.0 : 0.0);
    float ditherY = float((uv1_16.y & 256u) != 0u ? 1.0 : 0.0);
    vec2 ditheringAndMaskTinting = vec2(ditherX, ditherY);

    vec4 position = jitterVertexPosition(worldPos);

    float camDist = length(ViewPositionAndTime.xyz - worldPos);
    float fogIntensity = clamp((((camDist / fogAndDist.z) + RenderChunkFogAlpha.x) - fogAndDist.x) / (fogAndDist.y - fogAndDist.x), 0.0, 1.0);
    // Atmospheric fog: color matches the sky horizon, intensity adds height,
    // horizon and rain fog to the engine's distance fog. lightmapY is used as sky exposure.
    environment env = calculateEnvironment(
        TimeOfDay.x,
        FogColor.rgb,
        vec3(FogAndDistanceControl.xy, RenderDistance.x),
        DimensionID.x);
    vec3 fogViewDir = (worldPos - ViewPositionAndTime.xyz) / max(camDist, 0.0001);
    fogIntensity = atmosphericFogIntensity(env, fogIntensity, camDist / fogAndDist.z, worldPos.y, fogViewDir.y, lightmapY);
    vec4 fog = vec4(atmosphericFogColor(env, fogViewDir, SkyColor.rgb, FogColor.rgb), fogIntensity);

#if defined(TRANSPARENT)
    if (a_color0.a < 0.95) {
        color.a = mix(a_color0.a, 1.0, clamp(camDist / FogAndDistanceControl.w, 0.0, 1.0));
    }
#endif

    v_clipPosition = position;
    v_worldPosition = vec4(worldPos, env.rainFactor);   // w carries the rain factor for water shading
    v_color0 = color;
    // z = 1 for water. Water is the only block with a translucent vertex alpha
    // (the engine uses the same test above for its distance fade).
    float isWater = 0.0;
#if defined(TRANSPARENT)
    if (a_color0.a < 0.95 && !env.underwater && !env.isNether && !env.isEnd) {
        isWater = 1.0;
    }
#endif
    v_ditheringAndMaskTinting = vec3(ditheringAndMaskTinting, isWater);
    v_fog = fog;
    v_lightmapUV = lightmapUV;
    // Light color (xyz) and block light warmth (w); the fragment shader applies them to the engine lightmap.
    v_lightTint = vec4(skyLightTint(env, MoonPhase.x), blockLightWarmth(env));
    v_texcoord0 = uv0;
    v_worldPos = worldPos;
    gl_Position = position;
}
