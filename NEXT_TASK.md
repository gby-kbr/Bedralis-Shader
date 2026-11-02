# Next Task

Last phase: 03 - Celestial system (merged to main; not compiled, not tested in game).
Last work: sun/moon tint in `SunMoon`, star visibility in `Stars`, new `celestial.h`.

Unfinished:
- `moonlightFromPhase()` is unused until Phase 05.
- Thunderstorm sky (needs thunder detection; `WeatherID` meaning not verified, do not guess).
- End sky (planned for Phase 10).
- Smooth underwater transition.

Files / functions in progress: none open. Phase 04 starts at `source/origin/function/fog.h` (empty) and the fog handling in `RenderChunk`, `Actor`, `Weather` and `Clouds`.

Remaining errors: none known, but nothing has been compiled by the real material compiler yet.

Unmet dependencies:
- Material build tooling (`../../tools/data/materials` referenced by `project.json` is missing).
- In-game confirmation of: `DimensionID.x` values (0/1/2), `TimeOfDay` phase (code assumes 0 = noon), `FogAndDistanceControl` semantics used by `detectRain`/`detectUnderwater`, sky dome being camera-centered, and `a_color0.x` direction (0 zenith, 1 horizon), `MoonPhase.x` range (0 = full moon), and that `sunDir` matches the rendered sun (used to tell sun from moon in `SunMoon`).

Concrete next steps:
1. Phase 04 - Atmospheric system: distance, height, horizon and weather fog in `fog.h`, and match fog color to the Phase 02 sky colors.
2. Re-check sunrise/sunset glow side and sun/moon tint once `sunDir` is verified in game.
