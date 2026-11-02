# Next Task

Last phase: 02 - Sky color system (merged to main; not compiled, not tested in game).
Last work: per-vertex sky colors in `Sky/vertex.sc` using `calculateSkyColor()`, plus a fix for reversed `smoothstep` edges in `detection.h`.

Unfinished:
- Thunderstorm sky (needs thunder detection; `WeatherID` meaning not verified, do not guess).
- End sky (planned for Phase 10).
- Smooth underwater transition.

Files / functions in progress: none open. Next phase starts at `source/origin/function/sky.h` and the `SunMoon`/`Stars` materials.

Remaining errors: none known, but nothing has been compiled by the real material compiler yet.

Unmet dependencies:
- Material build tooling (`../../tools/data/materials` referenced by `project.json` is missing).
- In-game confirmation of: `DimensionID.x` values (0/1/2), `TimeOfDay` phase (code assumes 0 = noon), `FogAndDistanceControl` semantics used by `detectRain`/`detectUnderwater`, sky dome being camera-centered, and `a_color0.x` direction (0 zenith, 1 horizon).

Concrete next steps:
1. Phase 03 - Celestial system: sun and moon rendering/glow, moon phases, stars, and verify `sunDir` matches where the sun texture is actually drawn.
2. Re-check the sunrise/sunset glow side once `sunDir` is verified.
