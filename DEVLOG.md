# Bedralis Shader - Development Log

## 2026-11-02 - Phase 04: Atmospheric system

Branch: `phase-04-atmospheric-system` (off main at `129e1af`). Merge status: see `git log` on main.

Changes:
- `source/origin/function/fog.h`: new `combineFog`, `atmosphericFogColor`, `atmosphericFogIntensity`.
- `source/origin/configuration.h`: fog macros.
- `source/materials/RenderChunk/vertex.sc`, `Actor/vertex.sc`, `Weather/vertex.sc`: atmospheric fog color (and intensity for terrain and actors).
- `source/materials/RenderChunk/fragment.sc`: fog mix uses `v_fog.rgb`.

Problems found:
- Fog is computed per vertex in the engine shaders, and the RenderChunk fragment read the `FogColor` uniform directly, which would have bypassed any custom fog color.
- A variable named `flat` is a reserved GLSL word; renamed to `alongHorizon` before commit.
- First tuning gave about 20% haze at half render distance in clear weather; strengths lowered (height 0.30 to 0.22, horizon 0.25 to 0.20).

Build / test results:
- Real shader compile: NOT run (no compiler or GLSL validator available).
- Python port of the fog math: no value outside 0..1 and fog is monotonic with distance. Clear midday extra fog about 0.15 / 0.27 / 0.35 at 50% / 80% / 100% distance; height fog 0.22 at y=64 and 0.03 at y=120; rain adds about 0.09 at 20% distance on top of the engine fog.
- Preview rendered (terrain to fog over distance: midday, sunset, night, clear and rain). The engine fog start/end used in the preview come from the `detectRain` constants (clear 0.55..1.0, rain 0.23..0.70); real values were not measured.
- In-game test: NOT run.

Assumptions still unverified: the RenderChunk vertex world position is absolute (consistent with the engine's own `camDist` use); `v_lightmapUV.y` is the sky light; the `FogControl` uniform used by `Actor` carries the same data as `FogAndDistanceControl`. The `FogAndDistanceControl.json` added in Phase 02 may duplicate a definition from the missing `tools/data/materials`.

## 2026-11-02 - Phase 03: Celestial system

Branch: `phase-03-celestial-system` (off main at `63a292b`). Merge status: see `git log` on main.

Changes:
- `source/origin/function/celestial.h`: new sun/moon tint, moonlight-from-phase and star visibility functions.
- `source/origin/configuration.h`: celestial macros.
- `source/materials/SunMoon/vertex.sc` and `fragment.sc`: per-body tint computed in the vertex shader and applied in the fragment shader.
- `source/materials/Stars/vertex.sc`: star alpha scaled by night visibility.

Problems found:
- Sun and moon share one material, so the body must be identified. The vertex direction is compared with the computed `sunDir` (smooth transition, no hard switch). This relies on `sunDir` matching the engine's sun position, which is still unverified.
- No verified total-time uniform for star twinkle, so twinkle was skipped.

Build / test results:
- Real shader compile: NOT run (no compiler or GLSL validator available).
- Python port of the math: 960 time/rain samples, no value out of range; stars are 0 at noon and dusk, 1 at midnight, 0.1 at midnight in full rain; moonlight by phase 1.00 (full) to 0.25 (new).
- Timeline preview rendered (sun/moon tint and star visibility, clear and rain). Not an in-game screenshot.
- In-game test: NOT run.

## 2026-11-02 - Phase 02: Sky color system

Branch: `phase-02-sky-color-system` (off main at `023dcb1`). Merge status: see `git log` on main.

Changes:
- `source/origin/function/sky.h`: new `skyDesaturate`, `overworldSkyZenith`, `overworldSkyHorizon`, `calculateSkyColor`.
- `source/origin/configuration.h`: sky palette and strength macros.
- `source/materials/Sky/vertex.sc`: now declares its uniforms and computes the sky color per vertex.
- `source/common/uniforms/FogAndDistanceControl.json`: new uniform definition.
- `source/origin/function/detection.h`: fixed reversed `smoothstep` edges from Phase 01.

Problems found:
- Phase 01 used `smoothstep(0.05, -0.05, x)`, undefined in GLSL when edge0 >= edge1. Fixed with `1.0 - smoothstep(-0.05, 0.05, x)`.
- `Sky/vertex.sc` used `SkyColor` and `FogColor` without declaring them; the declarations are now explicit.
- Sun glow pushed channels above 1.0 (max 1.084 in the preview); the result is now clamped.

Build / test results:
- Real shader compile: NOT run (no compiler or GLSL validator available).
- Palette preview rendered with a Python port of the same math (sunrise, midday, sunset, night, rain day/night, Nether). The preview approximates the engine's own `SkyColor`/`FogColor` (darkened at night), so it is a palette check, not an in-game screenshot.
- In-game test: NOT run.

Assumptions still unverified: the sky dome is centered on the camera (world position used as view direction); `a_color0.x` is 0 at the zenith and 1 at the horizon; `sunDir` matches the rendered sun position (Phase 03 will check).
Not done: thunderstorm sky (no thunder detection), End sky (Phase 10).

## 2026-11-02 - Phase 01: Core architecture & environment detection

Branch: `phase-01-environment-detection` (off `origin/main` at `68745b2`). Merge status: MERGED to main as `023dcb1`.

Changes:
- `source/origin/configuration.h`: added `PI`, `SUN_PATH_TILT/YAW`, `MOON_PATH_TILT/YAW`, `DIM_OVERWORLD/NETHER/END`.
- `source/origin/function/utility.h`: added `degToRad()` and `rotate2()`.
- `source/origin/function/detection.h`: replaced `rmat2`/`mul` rotation with `rotate2`; added `detectDimension()`, daylight/night/dawn/dusk factors, `isNether`/`isEnd`, and `calculateEnvironment()`.

Problems found:
- `detection.h` could not compile: it used symbols that were defined nowhere.
- Repository has no build tooling and no GLSL validator is available in the session.

Solution:
- Defined the missing symbols; avoided `mat2` to stay portable across the multiplatform profile.

Build / test results:
- Real shader compile: NOT run (no compiler for this material format available).
- Logic check: day/night/dawn/dusk weights were reproduced in a Python script and checked across 15 time-of-day samples; transitions are smooth and dawn/dusk are mutually exclusive.
- In-game test: NOT run.

Preview: none (no renderer available).
Commit hash: see `git log` on the branch.
