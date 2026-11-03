#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Global constants and user-tunable settings for Bedralis.
// Keep this file free of functions; shared helpers live in function/utility.h.

#define PI 3.14159265359

// Sun and moon orbit orientation, in degrees.
// TILT rotates the orbit around the east-west axis, YAW around the vertical axis.
#define SUN_PATH_TILT 0.0
#define SUN_PATH_YAW 0.0
#define MOON_PATH_TILT 0.0
#define MOON_PATH_YAW 0.0

// Dimension IDs as reported by the DimensionID uniform (x component).
// NOTE: unverified in-game, see NEXT_TASK.md.
#define DIM_OVERWORLD 0
#define DIM_NETHER 1
#define DIM_END 2

// ---- Sky palette (Phase 02) ----
// Desaturated, natural colors. Zenith = top of the dome, horizon = at the fog line.
#define SKY_DAY_ZENITH   vec3(0.30, 0.47, 0.72)
#define SKY_DAY_HORIZON  vec3(0.62, 0.72, 0.82)
#define SKY_NIGHT_ZENITH  vec3(0.015, 0.02, 0.045)
#define SKY_NIGHT_HORIZON vec3(0.04, 0.05, 0.09)
#define SKY_DAWN_ZENITH   vec3(0.30, 0.38, 0.58)
#define SKY_DAWN_HORIZON  vec3(0.85, 0.58, 0.45)
#define SKY_DUSK_ZENITH   vec3(0.26, 0.30, 0.50)
#define SKY_DUSK_HORIZON  vec3(0.88, 0.52, 0.38)
#define SKY_RAIN_DAY      vec3(0.45, 0.48, 0.52)
#define SKY_RAIN_NIGHT    vec3(0.04, 0.045, 0.06)
#define SKY_SUN_GLOW      vec3(0.90, 0.50, 0.30)

#define SKY_RAIN_STRENGTH 0.85   // how far the sky moves toward the rain palette at full rain
#define SKY_GLOW_STRENGTH 0.35   // dawn/dusk glow toward the sun
#define SKY_BIOME_TINT 0.25      // amount of vanilla SkyColor/FogColor kept, so biome fog still shows
#define NETHER_SKY_DARKEN 0.90
#define NETHER_SKY_DESATURATE 0.20

// ---- Celestial bodies (Phase 03) ----
#define SUN_TINT_LOW   vec3(1.00, 0.62, 0.38)   // sun near the horizon
#define SUN_TINT_HIGH  vec3(1.00, 0.96, 0.88)   // sun high in the sky
#define MOON_TINT      vec3(0.80, 0.86, 0.95)
#define SUN_RAIN_DIM   0.60   // how much the sun disc is dimmed at full rain
#define MOON_RAIN_DIM  0.70
#define STAR_RAIN_FADE 0.90   // how much stars fade at full rain
#define MOON_NEW_LIGHT 0.25   // moonlight strength at new moon (full moon = 1.0)

// ---- Atmospheric fog (Phase 04) ----
#define HEIGHT_FOG_STRENGTH  0.22   // extra fog near the ground at far distance
#define HEIGHT_FOG_BASE      64.0   // world height (blocks) below which height fog is at full strength
#define HEIGHT_FOG_FALLOFF   0.035  // how fast height fog thins out above the base
#define HORIZON_FOG_STRENGTH 0.20   // extra fog when looking along the horizon at far distance
#define RAIN_FOG_STRENGTH    0.55   // extra fog at full rain

// ---- Lighting (Phase 05) ----
// The engine lightmap stays the base; these tints are multiplied on top of it.
#define LIGHT_DAY_TINT    vec3(1.00, 0.98, 0.95)
#define LIGHT_DAWN_TINT   vec3(1.00, 0.82, 0.66)
#define LIGHT_DUSK_TINT   vec3(1.00, 0.78, 0.62)
#define LIGHT_NIGHT_TINT  vec3(0.78, 0.86, 1.00)
#define LIGHT_BLOCK_TINT  vec3(1.00, 0.82, 0.58)   // torch / block light
#define LIGHT_BLOCK_WARMTH_DAY   0.15   // how strongly block light is tinted warm in daylight
#define LIGHT_BLOCK_WARMTH_NIGHT 0.60   // ... and at night
#define LIGHT_RAIN_DESAT   0.50   // sky light desaturation at full rain
#define LIGHT_RAIN_DIM     0.12   // sky light dimming at full rain
#define LIGHT_NEWMOON_DIM  0.12   // night sky light dimming at new moon (full moon = none)

// ---- Water (Phase 06) ----
#define WATER_SATURATION       0.85   // 1.0 keeps the engine water color, lower = more neutral
#define WATER_TINT             vec3(0.16, 0.30, 0.36)
#define WATER_TINT_MIX         0.12
#define WATER_ALPHA_SCALE      0.60   // < 1 makes water clearer (the engine alpha is scaled by this)
#define WATER_FRESNEL_ALPHA    0.50   // extra opacity at grazing angles
#define WATER_F0               0.03   // base reflectivity of water
#define WATER_REFLECT_STRENGTH 0.90
#define WATER_SKY_ZENITH_RATIO vec3(0.60, 0.72, 0.95)   // zenith color relative to the horizon color
#define WATER_SUN_STRENGTH     1.60
#define WATER_MOON_STRENGTH    0.60
#define WATER_WAVE_STRENGTH    1.0    // multiplier for wave slope
#define WATER_SWELL_FADE_START  60.0   // each wave layer fades out with distance to avoid shimmer
#define WATER_SWELL_FADE_END   220.0
#define WATER_WAVE_FADE_START  24.0
#define WATER_WAVE_FADE_END    90.0
#define WATER_DETAIL_FADE_START 12.0
#define WATER_DETAIL_FADE_END  50.0

// Experimental: move the water surface vertices with the swell (see RenderChunk vertex).
// Off by default because it cannot be tested without the game: it may show gaps along
// shores or chunk borders. Set WATER_VERTEX_WAVES to 1 to try it; WATER_VERTEX_WAVE_SCALE
// scales the height (max 0.085 blocks at 1.0).
#define WATER_VERTEX_WAVES 0
#define WATER_VERTEX_WAVE_SCALE 1.0
#define WATER_RAIN_RIPPLE      0.12   // extra small ripples at full rain
#define WATER_UNDERWATER_DESAT 0.25   // underwater fog color desaturation

// ---- Clouds (Phase 07) ----
// Cirrus clouds are painted on the sky from a noise texture, so the shader needs only
// three texture reads and no procedural noise. Texture: assets/textures/environment/
// CirrusTexture.png (made by scripts/generate_cirrus_texture.py), bound as the buffer
// "noisetexture". R = strands, G = veil mass, B = fine detail.
#define CIRRUS_ENABLED 1
#define CIRRUS_SCALE 0.8           // bigger = smaller clouds
#define CIRRUS_DOME 0.35           // 0 = flat plane (strong perspective), higher = flatter dome
#define CIRRUS_ROTATION 18.0       // degrees: direction the strands and the wind run
#define CIRRUS_SPEED 0.002         // wind speed in texture tiles per second
#define CIRRUS_COVERAGE 0.50       // 0..1, how much of the veil mass shows
#define CIRRUS_SOFTNESS 0.60       // softness of veil edges
#define CIRRUS_ENV_LOW 0.46        // large clear / cloudy regions (LOW must stay below HIGH);
#define CIRRUS_ENV_HIGH 0.62       // closer values give sharper regions
#define CIRRUS_WARP 0.06           // how much the veil bends the strands
#define CIRRUS_VEIL_BASE 0.35      // veil density where there are no strands
#define CIRRUS_STRAND_LOW 0.80     // strand threshold (LOW below HIGH): higher = fewer strands
#define CIRRUS_STRAND_HIGH 0.97
#define CIRRUS_DETAIL 0.35         // fine detail inside the veil
#define CIRRUS_WISP_REACH 0.32     // how far strands stray out of the veil
#define CIRRUS_OPACITY 0.88
#define CIRRUS_SHADING 0.55        // thick parts shift toward the shade color
#define CIRRUS_SUN_GLOW 0.55       // silver lining near the sun
#define CIRRUS_TWILIGHT_TINT 0.70  // how much the sun color tints the clouds (1 = fully)
#define CIRRUS_COLOR_LIGHT vec3(1.00, 1.00, 1.00)   // lit parts
#define CIRRUS_COLOR_SHADE vec3(0.60, 0.68, 0.82)   // thick, shaded parts
#define CIRRUS_COLOR_NIGHT vec3(0.12, 0.14, 0.20)   // at night
// The vanilla block clouds (Clouds material). 0 hides them so only the cirrus remain.
// If both are off there are no clouds at all.
#define VANILLA_CLOUDS 0
// Reflect the cirrus in water (3 more texture reads, water pixels only).
#define WATER_REFLECT_CLOUDS 1

#endif

// ---------------------------------------------------------------------------
// Weather (Phase 08)
// ---------------------------------------------------------------------------
#define WEATHER_RAIN_SLANT 1.5         // 1.0 = engine slant, higher = rain leans more with the wind
#define WEATHER_RAIN_WIDTH 0.28        // rain streak width scale (the pack texture is already thin)
#define WEATHER_RAIN_LENGTH 0.50       // rain streak length scale
#define WEATHER_SNOW_SIZE 0.50         // snowflake size scale
#define WEATHER_RAIN_OPACITY 0.85      // rain streak opacity scale
#define WEATHER_SNOW_OPACITY 0.95      // snowflake opacity scale
#define WEATHER_RAIN_SKY_BLEND 0.35    // how much rain takes the sky/fog color (soft, misty look)
#define WEATHER_SNOW_SOFT 0.0          // 0 = pack flake texture, 1 = soft round speck
#define WEATHER_RAIN_DARKEN 0.85       // rain brightness at full daylight (lower = darker streaks)
