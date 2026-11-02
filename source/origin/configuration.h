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

#endif
