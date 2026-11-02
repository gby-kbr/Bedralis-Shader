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

#endif
