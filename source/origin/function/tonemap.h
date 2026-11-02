#ifndef TONEMAP_H
#define TONEMAP_H

#include "origin/configuration.h"
#include "origin/function/utility.h"

vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b) {
  float l0 = ((P - m) * l) / a;
  float L0 = m - m / a;
  float L1 = m + (1.0 - m) / a;
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

vec3 colorCorrection(vec3 col) {
#ifdef EXPOSURE
  col *= EXPOSURE;
#endif

  const float P = 1.0;
  const float a = 1.0;
  const float m = 0.22;
  const float l = 0.4;
  const float c = 1.33;
  const float b = 0.0;
  col = uchimura(col, P, a, m, l, c, b);

  col = pow(col, vec3_splat(1.0 / GAMMA));

#ifdef SATURATION
  col = mix(vec3_splat(luminance(col)), col, SATURATION);
#endif

#ifdef TINT
  col *= mix(TINT_LOW, TINT_HIGH, col);
#endif

  return col;
}

#endif
