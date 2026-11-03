#ifndef TONEMAP_H
#define TONEMAP_H

#include "origin/configuration.h"

// Uchimura 2017, "HDR theory and practice"
// Math: https://www.desmos.com/calculator/gslcdxvipg
// Same function and default values as the tonemap used in newb-aerotion.
vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b) {
  float l0 = ((P - m) * l) / a;
  float S0 = m + l0;
  float S1 = m + a * l0;
  float C2 = (a * P) / (P - S1);
  float CP = -C2 / P;

  vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
  vec3 w2 = vec3(step(m + l0, x));
  vec3 w1 = vec3(1.0 - w0 - w2);

  vec3 T = vec3(m * pow(x / m, vec3_splat(c)) + b);
  vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
  vec3 L = vec3(m + a * (x - m));

  return T * w0 + L * w1 + S * w2;
}

// Final grade: exposure, Uchimura tonemap, saturation.
vec3 bedralisGrade(vec3 c) {
#if GRADE_ENABLED
  c = max(c * GRADE_EXPOSURE, vec3(0.0, 0.0, 0.0));
  c = uchimura(c, UCHIMURA_MAX, UCHIMURA_CONTRAST, UCHIMURA_LINEAR_START,
               UCHIMURA_LINEAR_LENGTH, UCHIMURA_BLACK, UCHIMURA_PEDESTAL);
  float luma = dot(c, vec3(0.299, 0.587, 0.114));
  c = mix(vec3(luma, luma, luma), c, GRADE_SATURATION);
  return clamp(c, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));
#else
  return c;
#endif
}

#endif
