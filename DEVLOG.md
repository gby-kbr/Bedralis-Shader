# Bedralis Shader - Development Log

## 2026-11-02 - Phase 01: Core architecture & environment detection

Branch: `phase-01-environment-detection` (off `origin/main` at `68745b2`). Merge status: NOT MERGED, awaiting review.

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
