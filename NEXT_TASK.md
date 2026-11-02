# Next Task

Last phase: 04 - Atmospheric system (merged to main; not compiled, not tested in game).
Last work: atmospheric fog in `fog.h`, wired into `RenderChunk`, `Actor` and `Weather`.

Unfinished:
- Thunder fog (needs thunder detection).
- Performance watch: `calculateEnvironment()` runs per vertex in `RenderChunk`; audit in Phase 11 (or move shared work out).
- `moonlightFromPhase()` is unused until Phase 05.
- Thunderstorm sky (needs thunder detection; `WeatherID` meaning not verified, do not guess).
- End sky (planned for Phase 10).
- Smooth underwater transition.

Files / functions in progress: none open. Phase 05 starts at `source/origin/function/lighting.h` (empty) and the lightmap/lighting code in `RenderChunk` and `Actor`.

Remaining errors: none known, but nothing has been compiled by the real material compiler yet.

Unmet dependencies:
- Material build tooling (`../../tools/data/materials` referenced by `project.json` is missing).
- In-game confirmation of: `DimensionID.x` values (0/1/2), `TimeOfDay` phase (code assumes 0 = noon), `FogAndDistanceControl` semantics used by `detectRain`/`detectUnderwater`, sky dome being camera-centered, and `a_color0.x` direction (0 zenith, 1 horizon), `MoonPhase.x` range (0 = full moon), and that `sunDir` matches the rendered sun (used to tell sun from moon in `SunMoon`); RenderChunk world position is absolute; `v_lightmapUV.y` is the sky light; `FogControl` (Actor) equals `FogAndDistanceControl`; `FogAndDistanceControl.json` does not duplicate a definition from the missing toolchain data.

Concrete next steps:
1. Phase 05 - Lighting system: sunlight, ambient, block light, directional light, night/weather lighting and transitions in `lighting.h`; use `moonlightFromPhase()` for night light.
2. Re-check sunrise/sunset glow side, sun/moon tint and fog tuning once the shader can be tested in game.
