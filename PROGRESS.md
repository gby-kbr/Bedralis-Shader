# Bedralis Shader - Progress

Status values: NOT STARTED, IN PROGRESS, TESTING, COMPLETED, MERGED.

| Phase | Name | Status |
|-------|------|--------|
| 01 | Core architecture & environment detection | MERGED (not compiled / not tested in game) |
| 02 | Sky color system | MERGED (not compiled / not tested in game) |
| 03 | Celestial system | MERGED (not compiled / not tested in game) |
| 04 | Atmospheric system | MERGED (not compiled / not tested in game) |
| 05 | Lighting system | NOT STARTED |
| 06 | Water system | NOT STARTED |
| 07 | Cloud system | NOT STARTED |
| 08 | Weather system | NOT STARTED |
| 09 | Advanced visual effects | NOT STARTED |
| 10 | Realistic The End outer space sky | NOT STARTED |
| 11 | Final optimization & quality control | NOT STARTED |

## Phase 04 details

Done:
- `fog.h`: `atmosphericFogColor()` (same function as the sky horizon, so terrain fades into the sky; vanilla color underwater and in The End) and `atmosphericFogIntensity()` (height, horizon and rain fog on top of the engine's distance fog; Overworld only).
- `RenderChunk`, `Actor`, `Weather` vertex shaders: `v_fog` now carries the atmospheric color; `RenderChunk` fragment uses `v_fog.rgb` instead of the `FogColor` uniform.
- `RenderChunk` uses the lightmap sky value as sky exposure so caves do not get sky-colored haze.
- `configuration.h`: `HEIGHT_FOG_*`, `HORIZON_FOG_STRENGTH`, `RAIN_FOG_STRENGTH`.
- Fixed before commit: a variable named `flat` (reserved word in GLSL) was renamed.

Not done / deferred:
- `Weather` only gets the fog color; rain streak fog intensity stays the engine's.
- `Actor` and `Weather` have no sky exposure value (exposure = 1.0 for `Actor`); the extra fog is small at the short distances where this matters.
- Clouds keep their vanilla distance fade (Phase 07).
- Thunder fog: needs thunder detection.
- Performance: `calculateEnvironment()` now runs per vertex for terrain; check in Phase 11.

## Phase 03 details

Done:
- `celestial.h`: `sunDiscTint`, `moonDiscTint`, `celestialBodyTint`, `moonlightFromPhase`, `starVisibility`.
- `SunMoon` vertex/fragment: sun tinted warm near the horizon and white when high, moon tinted cool, both dimmed in rain.
- `Stars` vertex: star alpha scaled by night visibility, fading out in rain.
- `configuration.h`: `SUN_TINT_*`, `MOON_TINT`, `*_RAIN_DIM`, `STAR_RAIN_FADE`, `MOON_NEW_LIGHT`.

Not done / deferred:
- The engine places the sun/moon quads and chooses the moon phase cell, so the shader cannot move them or redraw phases; only color and brightness are controlled.
- `moonlightFromPhase()` is ready but not used yet (night lighting is Phase 05).
- Star twinkle: no verified total-time uniform (`ElapsedFrameTime` looks like a frame delta), so it was not added.
- Extra sun glow disc: the dawn/dusk glow already lives in the sky (Phase 02); a separate halo would need extra geometry.

## Phase 02 details

Done:
- `sky.h`: `calculateSkyColor()` with day / dawn / dusk / night palettes, rain overcast, dawn/dusk sun glow, Nether (engine fog color, darker and less saturated), horizon gradient, and a small share of the engine `SkyColor`/`FogColor` to keep biome fog tint.
- `configuration.h`: palette and strength macros (`SKY_*`, `NETHER_*`).
- `Sky/vertex.sc`: colors are computed per vertex through `calculateEnvironment()` + `calculateSkyColor()`; fragment stays pass-through.
- `common/uniforms/FogAndDistanceControl.json`: new uniform definition.
- Phase 01 fix: `smoothstep` had reversed edges (undefined in GLSL); now `1.0 - smoothstep(...)`.

Not done / deferred:
- Thunderstorm sky: needs thunder detection (still unverified data source).
- End sky: `calculateSkyColor()` returns the engine color for the End; the real work is Phase 10 in the `EndSky` material.
- Underwater sky is not handled (the sky is not visible underwater).

## Phase 01 details

Done (merged to main in `023dcb1`):
- `configuration.h`: `PI`, sun/moon orbit macros, `DIM_*` dimension IDs.
- `utility.h`: `degToRad`, `rotate2` (scalar rotation, no `mat2`, so it is portable across GLSL/HLSL/Metal).
- `detection.h`: extended `environment` struct, `detectDimension`, smooth daylight/night/dawn/dusk weights, and `calculateEnvironment()` as the single entry point.

Not done yet:
- Thunder detection (no verified data source yet).
- Smooth underwater transition (needs per-frame history, not available).
- Wiring `calculateEnvironment()` into any material (no material includes `detection.h` yet).
- Real shader compile and in-game test.

## Repository audit notes (baseline `68745b2`)

- Only `detection.h` had content. `configuration.h`, `utility.h`, `sky.h`, `fog.h`, `clouds.h`, `lighting.h`, `noise.h`, `tonemap.h`, `water.h`, `wave.h`, `weather.h` and `insert.sh` were empty; `godrays.h` was 1 byte.
- `detection.h` referenced undefined `PI`, `rmat2`, `degToRad`, `SUN_PATH_*`, `MOON_PATH_*`.
- All materials (`Sky`, `EndSky`, `Clouds`, `Stars`, `SunMoon`, `Weather`, `Actor`, `RenderChunk`) are still near-vanilla pass-through shaders.
- No build tooling in the repo (`project.json` points at `../../tools/data/materials`, which is not present).
