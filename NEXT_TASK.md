# Next Task

Last phase: 01 - Core architecture & environment detection (TESTING).
Last work: implemented the centralized environment state in `detection.h`, plus supporting definitions.
Commit / merge status: committed on branch `phase-01-environment-detection`; not pushed, not merged.

Unfinished:
- Thunder detection (`WeatherID` meaning is not verified; do not guess).
- Smooth underwater transition.
- Wire `calculateEnvironment()` into a material (start with `Sky/fragment.sc`).

Files / functions in progress: `source/origin/function/detection.h` (`calculateEnvironment`).

Remaining errors: none known, but nothing has been compiled by the real material compiler yet.

Unmet dependencies:
- Material build tooling (`../../tools/data/materials` is referenced by `project.json` but missing).
- In-game confirmation of: `DimensionID.x` values (0/1/2), `TimeOfDay` phase (code assumes 0 = noon), and `FogControl` semantics used by `detectRain`/`detectUnderwater`.

Concrete next steps:
1. Review and approve Phase 01 (open PR from the branch).
2. Verify the three assumptions above in game.
3. Then start Phase 02 (sky color system) using `environment` as input.
