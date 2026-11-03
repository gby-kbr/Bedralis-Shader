# Next Task

Last phase: 08 - Weather system (rain/snow look); before that 07 - Cirrus (merged to main; not compiled, not tested in game).
Last work: `weather.h` and the `Weather` material (thinner misty rain, soft snow, slant, alpha fog). Cirrus and water work from Phase 07 stay as they were.

Unfinished:
- Try `WATER_VERTEX_WAVES = 1` in game and check shores and chunk borders for gaps; tune `WATER_ALPHA_SCALE` (clarity) while looking at real seabeds.
- Tune `CIRRUS_COVERAGE`, `CIRRUS_SCALE`, `CIRRUS_SPEED` and colors against the real game; `VANILLA_CLOUDS 1` restores block clouds.
- Shadow integration and true N.L lighting (no shadow map or terrain normals).
- Thunder fog/lighting (needs thunder detection).
- Performance watch: `calculateEnvironment()` runs per vertex in `RenderChunk`; audit in Phase 11 (or move shared work out).
- Thunderstorm sky (needs thunder detection; `WeatherID` meaning not verified, do not guess).
- End sky (planned for Phase 10).
- Smooth underwater transition.

Files / functions in progress: none open. Phase 09 (advanced effects) starts from `godrays.h` and `tonemap.h` (both empty); check first what the pipeline allows (no scene color or depth buffer).

Remaining errors: none known, but nothing has been compiled by the real material compiler yet.

Unmet dependencies:
- Thunder detection (still no verified source); rain vs snow detection in `Weather` is unverified in game.
- Material build tooling (`../../tools/data/materials` referenced by `project.json` is missing).
- In-game confirmation of: `DimensionID.x` values (0/1/2), `TimeOfDay` phase (code assumes 0 = noon), `FogAndDistanceControl` semantics used by `detectRain`/`detectUnderwater`, the Sky mesh behaving like the Aerotion full-screen quad, packed COLOR0..3 varyings, noise buffer slots 9 (Sky) / 7 (RenderChunk) not colliding, `MoonPhase.x` range (0 = full moon), and that `sunDir` matches the rendered sun (used to tell sun from moon in `SunMoon`); RenderChunk world position is absolute; `v_lightmapUV.y` is the sky light; `FogControl` (Actor) equals `FogAndDistanceControl`; `v_lightmapUV` is (block, sky); the light tint does not over-darken the engine lightmap; `ViewPositionAndTime.w` is time in seconds; water is identified by vertex alpha < 0.95 in the TRANSPARENT path; the vec3 `v_ditheringAndMaskTinting` varying is accepted by the compiler.

Concrete next steps:
1. Phase 09 - Advanced visual effects: pick only what is possible without a depth or color buffer (tonemap in material shaders, sun glow/god ray approximation, vignette-free grading); keep mobile cost low.
2. Re-check sunrise/sunset glow side, sun/moon tint, fog, light tint and water tuning once the shader can be tested in game.
