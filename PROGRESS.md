# Bedralis Shader - Progress

Status values: NOT STARTED, IN PROGRESS, TESTING, COMPLETED, MERGED.

| Phase | Name | Status |
|-------|------|--------|
| 01 | Core architecture & environment detection | TESTING |
| 02 | Sky color system | NOT STARTED |
| 03 | Celestial system | NOT STARTED |
| 04 | Atmospheric system | NOT STARTED |
| 05 | Lighting system | NOT STARTED |
| 06 | Water system | NOT STARTED |
| 07 | Cloud system | NOT STARTED |
| 08 | Weather system | NOT STARTED |
| 09 | Advanced visual effects | NOT STARTED |
| 10 | Realistic The End outer space sky | NOT STARTED |
| 11 | Final optimization & quality control | NOT STARTED |

## Phase 01 details

Done (on branch `phase-01-environment-detection`, not merged):
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
