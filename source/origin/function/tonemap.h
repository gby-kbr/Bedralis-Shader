#ifndef TONEMAP_H
#define TONEMAP_H

#include "origin/configuration.h"

// Gentle final grade: saturation, contrast and a soft highlight shoulder.
// Near neutral with the defaults; it exists mainly to avoid hard clipping of bright
// values (sun glow, lit snow, sunset sky) and to give every material the same look.
vec3 bedralisGrade(vec3 c) {
#if GRADE_ENABLED
  c *= GRADE_EXPOSURE;

  float luma = dot(c, vec3(0.299, 0.587, 0.114));
  c = mix(vec3(luma, luma, luma), c, GRADE_SATURATION);
  c = (c - 0.5) * GRADE_CONTRAST + 0.5;
  c = max(c, vec3(0.0, 0.0, 0.0));

  // Highlight shoulder: identity below GRADE_SHOULDER, smooth roll-off toward 1 above it.
  float room = 1.0 - GRADE_SHOULDER;
  vec3 over = max(c - GRADE_SHOULDER, vec3(0.0, 0.0, 0.0));
  c = c - over + room * (vec3(1.0, 1.0, 1.0) - exp(-over / room));
  return clamp(c, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));
#else
  return c;
#endif
}

#endif
