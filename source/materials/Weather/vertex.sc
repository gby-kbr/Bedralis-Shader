$input a_position, a_color0, a_texcoord0
#ifdef INSTANCING
    $input i_data1, i_data2, i_data3
#endif

$output v_color0, v_fog, v_occlusionHeight, v_occlusionUV, v_texcoord0, v_worldPos

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/FogUtil.dragonh>
#include <MinecraftRenderer.Materials/TAAUtil.dragonh>
#include "origin/function/fog.h"
#include "origin/function/weather.h"

uniform vec4 Dimensions;
uniform vec4 FogAndDistanceControl;
uniform vec4 FogColor;
uniform vec4 PositionBaseOffset;
uniform vec4 PositionForwardOffset;
uniform vec4 UVOffsetAndScale;
uniform vec4 Velocity;
uniform vec4 ViewPosition;
uniform vec4 SkyColor;
uniform vec4 TimeOfDay;
uniform vec4 DimensionID;
uniform vec4 RenderDistance;

#if BGFX_SHADER_LANGUAGE_GLSL
    #if BGFX_SHADER_LANGUAGE_GLSL < 130
    float trunc(float _x) { return float(int(_x)); }
    #endif
float fmod(float _a, float _b) { return _a - _b * trunc(_a / _b); }
vec3 fmod(vec3 _a, vec3 _b) { return vec3(fmod(_a.x, _b.x), fmod(_a.y, _b.y), fmod(_a.z, _b.z)); }
#endif

void main() {
    float spriteSelector = a_color0.x * 255.0;

    vec2 texcoord0 = UVOffsetAndScale.xy + (a_texcoord0 * UVOffsetAndScale.zw);
#if !defined(NO_VARIETY)
    texcoord0.x += spriteSelector * UVOffsetAndScale.z;
#endif

    vec3 particleBox = vec3(30.0, 30.0, 30.0);

    vec3 worldSpacePosition = fmod(a_position + PositionBaseOffset.xyz, particleBox);
    worldSpacePosition -= particleBox * 0.5;
    worldSpacePosition += PositionForwardOffset.xyz;

    // Occlusion lookup and fog direction use the position before any slant.
    vec3 occlusionPosition = worldSpacePosition;

    bool isRain = isRainParticle(UVOffsetAndScale, Dimensions.x);
    vec2 particleSize = weatherParticleSize(isRain, Dimensions.xy);
    vec3 velocity = Velocity.xyz;
    if (isRain) {
        velocity.x *= WEATHER_RAIN_SLANT;
        worldSpacePosition.x -= worldSpacePosition.y * velocity.x;
    }

    vec3 worldSpacePositionBottom = worldSpacePosition;
    vec3 worldSpacePositionTop = worldSpacePositionBottom + (velocity * particleSize.y);

    vec4 screenSpacePositionBottom = jitterVertexPosition(worldSpacePositionBottom);
    vec4 screenSpacePositionTop = jitterVertexPosition(worldSpacePositionTop);

    vec2 screenSpaceUpDirection = (screenSpacePositionTop.xy / screenSpacePositionTop.w) - (screenSpacePositionBottom.xy / screenSpacePositionBottom.w);
    vec2 screenSpaceRightDirection = normalize(vec2(-screenSpaceUpDirection.y, screenSpaceUpDirection.x));

    vec4 position = mix(screenSpacePositionTop, screenSpacePositionBottom, a_texcoord0.y);
    position.xy += (0.5 - a_texcoord0.x) * screenSpaceRightDirection * particleSize.x;

    vec2 occlusionUV = ((occlusionPosition.xz + ViewPosition.xz) * (1.0 / 64.0)) + vec2_splat(0.5);
    float occlusionHeight = (occlusionPosition.y + ViewPosition.y - 0.5) * (1.0 / 255.0);

    float fogIntensity = calculateFogIntensity(position.z, FogAndDistanceControl.z, FogAndDistanceControl.x, FogAndDistanceControl.y);
    // Only the fog color follows the atmosphere here; the intensity stays the engine's.
    // worldSpacePosition is camera-relative, so it doubles as the view direction.
    environment env = calculateEnvironment(
        TimeOfDay.x,
        FogColor.rgb,
        vec3(FogAndDistanceControl.xy, RenderDistance.x),
        DimensionID.x);
    vec3 fogViewDir = normalize(occlusionPosition);
    vec4 fog = vec4(atmosphericFogColor(env, fogViewDir, SkyColor.rgb, FogColor.rgb), fogIntensity);

    // v_color0 is unused by the fragment shader, so it carries our data instead:
    // x = 1 for rain, y = daylight, zw = quad UV.
    v_color0 = vec4(isRain ? 1.0 : 0.0, env.daylight, a_texcoord0);
    v_fog = fog;
    v_occlusionHeight = occlusionHeight;
    v_occlusionUV = occlusionUV;
    v_texcoord0 = texcoord0;
    v_worldPos = occlusionPosition;
    gl_Position = position;
}
