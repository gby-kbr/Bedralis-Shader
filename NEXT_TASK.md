# Next Task

Last phase: 05 - Lighting system (merged to main; not compiled, not tested in game).
Last work: light color tint in `lighting.h`, applied to the engine lightmap in `RenderChunk` and to `Actor`.

Unfinished:
- Shadow integration and true N.L lighting (no shadow map or terrain normals).
- Thunder fog/lighting (needs thunder detection).
- Performance watch: `calculateEnvironment()` runs per vertex in `RenderChunk`; audit in Phase 11 (or move shared work out).
- Thunderstorm sky (needs thunder detection; `WeatherID` meaning not verified, do not guess).
- End sky (planned for Phase 10).
- Smooth underwater transition.

Files / functions in progress: none open. Phase 06 starts at `source/origin/function/water.h` and `wave.h` (both empty) and the TRANSPARENT path in `RenderChunk`.

Remaining errors: none known, but nothing has been compiled by the real material compiler yet.

Unmet dependencies:
- Material build tooling (`../../tools/data/materials` referenced by `project.json` is missing).
- In-game confirmation of: `DimensionID.x` values (0/1/2), `TimeOfDay` phase (code assumes 0 = noon), `FogAndDistanceControl` semantics used by `detectRain`/`detectUnderwater`, sky dome being camera-centered, and `a_color0.x` direction (0 zenith, 1 horizon), `MoonPhase.x` range (0 = full moon), and that `sunDir` matches the rendered sun (used to tell sun from moon in `SunMoon`); RenderChunk world position is absolute; `v_lightmapUV.y` is the sky light; `FogControl` (Actor) equals `FogAndDistanceControl`; `FogAndDistanceControl.json` does not duplicate a definition from the missing toolchain data; `v_lightmapUV` is (block, sky); the light tint does not over-darken the engine lightmap.

Concrete next steps:
1. Phase 06 - Water system: water color, transparency, underwater rendering, waves, surface lighting and rain interaction in `water.h` / `wave.h`; keep water clear, not too blue or green.
2. Re-check sunrise/sunset glow side, sun/moon tint, fog and light tint tuning once the shader can be tested in game.
