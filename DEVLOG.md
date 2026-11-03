# Bedralis Shader - Development Log

## 2026-11-03 - Phase 07: Cirrus cloud system

Branch: `phase-07-cloud-system` (off main at `751fab4`). Merge status: see `git log` on main.

Request from the project owner: cirrus clouds, built from `buffers/noisetexture.json` and a noise texture, light and realistic, matching a reference screenshot (soft veil plus sparse fibrous wisps).

Changes:
- New: `clouds.h`, `CirrusTexture.png`, `scripts/generate_cirrus_texture.py`, `Sky/buffers/noisetexture.json`, `RenderChunk/buffers/noisetexture.json`.
- Changed: `Sky` vertex/fragment/varying.def (full-screen quad), `sky.h` (wind phase, env unpacking), `water.h` + `RenderChunk/fragment.sc` (cirrus reflection), `Clouds/vertex.sc` (vanilla clouds optional), `configuration.h`.
- Removed: `common/uniforms/FogAndDistanceControl.json`.

Decisions:
- The Phase 02 sky assumed a camera-centered dome and `a_color0.x` as a height gradient; neither was ever verified. The Sky is now a full-screen quad, a technique already proven in newb-aerotion, so cloud direction no longer depends on the mesh.
- Trig is done in the vertex shader and passed as packed varyings; the fragment shader only samples and mixes.
- Texture channels are rank-transformed to uniform 0..1, so a threshold equals a coverage fraction.

Provenance note: `CirrusTexture.png`, `scripts/` and an empty local branch of the same name already existed in the working copy when this phase began. I read the script fully, regenerated the texture from it (pixel-identical, tileable) and used it as-is; the final script is documented in its docstring.

Problems found:
- First render looked like cottage cheese: texture channels were isotropic blobs. Regenerated an anisotropic veil mass and sparser strands; warp taken from G only (B warp made jagged edges); added a low-frequency envelope tap so the veil is not uniform across the sky; reduced warp to stop curly strands; twilight orange toned down with `CIRRUS_TWILIGHT_TINT`.

Build / test results:
- Real shader compile: NOT run (no compiler available).
- Python port of the cloud math rendered on six scenes (midday up, midday horizon, sunset, night, rain, sunrise); no value outside 0..1; alpha max 0.88 (0.31 at night, 0 in rain); output at wind phase 0 and 4 is identical (seamless wrap); the Python parameters match `configuration.h`.
- Previews are approximations, not Minecraft screenshots.
- In-game test: NOT run.

Assumptions still unverified: the Sky mesh behaves like the Aerotion quad (`a_position.xzy` clamped to +-0.5); packed COLOR0..3 varyings are accepted; buffer register slots (9 Sky, 7 RenderChunk) do not collide; `ViewPositionAndTime.w` is seconds; `texture2DLod` and `SAMPLER2D_AUTOREG` work with the buffer's `sampler_state`.

## 2026-11-02 - Phase 06 refinement: clearer water and richer waves

Branch: `water-clarity-and-waves` (off main at `b02bdb7`). Merge status: see `git log` on main.

Changes:
- `source/origin/function/wave.h`: rewritten with swell, medium (domain-warped) and detail layers, per-layer distance fade, rain ripples, and `waterWaveHeight()` for optional vertex waves.
- `source/origin/configuration.h`: clearer water defaults, new fade distances, `WATER_VERTEX_WAVES` (0) and `WATER_VERTEX_WAVE_SCALE`.
- `source/materials/RenderChunk/vertex.sc`: optional vertex wave block, compiled out by default.

Build / test results:
- Real shader compile: NOT run (no compiler or GLSL validator available).
- Python port rendered on the same scenes as before: opacity range 0.57-0.99 became 0.41-0.88; max wave tilt 6.6 degrees clear and 8.4 in rain; no out-of-range values.
- The preview assumes a dark seabed, so it cannot judge real clarity; the engine's lit seabed will show through in game.
- Vertex waves: not tested (default off).
- In-game test: NOT run.

## 2026-11-02 - Phase 06: Water system

Branch: `phase-06-water-system` (off main at `8e45b3e`). Merge status: see `git log` on main.

Changes:
- `source/origin/function/wave.h`, `water.h`: new water normal and shading functions.
- `source/origin/function/detection.h`: `orbitDirection`, `sunDirection`, `moonDirection` (refactor, same results as before).
- `source/origin/function/fog.h`: underwater fog color through `underwaterFogColor()`.
- `source/materials/RenderChunk/vertex.sc`, `varying.def.sc`, `fragment.sc`: water flag, rain factor, and `shadeWater()` call.
- `source/origin/configuration.h`: water macros.

Decisions:
- Per the project owner: no reflection except sun/moon/sky (and clouds later). Screen-space reflection and object reflection were not attempted.
- No new varying semantics were added; the water flag and rain factor reuse free components of existing varyings (a vec2 became a vec3, and `v_worldPosition.w` was unused).

Problems found:
- A first patch attempt for `detection.h` failed on a trailing-space mismatch and the script still printed "patched"; caught by checking `git status` and the macro count, then redone and verified.
- A branch named `phase-06-water-system` already existed locally; it pointed at the same commit as main with no extra commits and was not on the remote, so it was reused.
- Moon glint was almost invisible at night; strength raised from 0.35 to 0.60. It stays subtle at steep angles because fresnel is low there.

Build / test results:
- Real shader compile: NOT run (no compiler or GLSL validator available).
- Python port of `waterNormal` and `shadeWater`, rendered as a water plane seen from 6 blocks up with the Phase 02 sky and Phase 04 fog (midday, sunset, night, rain): no out-of-range values; opacity 0.57 to 0.99; fresnel 0.03 to 0.98; max wave tilt 7.3 degrees (9.7 in rain); sun glint column visible at sunset; the sky/fog color match at the horizon was checked numerically (0.819, 0.626, 0.578 on both sides).
- Preview is a Python approximation (assumed water texture color and engine fog range), not an in-game screenshot.
- In-game test: NOT run.

Assumptions still unverified: `ViewPositionAndTime.w` is time in seconds; water is the only TRANSPARENT block with vertex alpha below 0.95; `v_worldPos` in `RenderChunk` is the absolute world position; changing the varying `v_ditheringAndMaskTinting` to a vec3 is accepted by the material compiler.

## 2026-11-02 - Phase 05: Lighting system

Branch: `phase-05-lighting-system` (off main at `35d9532`). Merge status: see `git log` on main.

Changes:
- `source/origin/function/lighting.h`: new `skyLightTint`, `blockLightWarmth`, `applyBedralisLight`.
- `source/origin/configuration.h`: lighting macros.
- `source/materials/RenderChunk/vertex.sc`, `varying.def.sc`, `fragment.sc`: new `v_lightTint` varying (TEXCOORD4); fragment applies it to the engine lightmap.
- `source/materials/Actor/vertex.sc`: sky light tint applied to `v_light`.

Problems found:
- Replacing the engine lightmap would risk breaking the Nether, The End, underwater and night vision, and nothing can be tested here, so the tint is applied on top of the lightmap instead.
- Terrain has no normals, so sun N.L lighting is not possible without per-pixel derivatives; the engine's baked face shading is kept.
- New moon dimming of 0.20 stacked on the engine's dark night lightmap looked too dark; lowered to 0.12.

Build / test results:
- Real shader compile: NOT run (no compiler or GLSL validator available).
- Python port of the math (480 time samples x clear/rain): no value outside 0..1, channels between 0.60 and 1.00. Noon clear tint (1.00, 0.98, 0.95); noon rain (0.87, 0.86, 0.85); night full moon (0.78, 0.86, 1.00); torch warmth stronger at night than at noon.
- Preview rendered (sky light and torch-lit tint across the day). It shows the multiplier only, not final pixels, since the engine lightmap is the other factor.
- In-game test: NOT run.

Assumptions still unverified: `v_lightmapUV` is (block light, sky light); `MoonPhase.x` runs 0..7 with 0 = full moon; the engine lightmap is the dominant brightness term so the tint does not over-darken it.

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
