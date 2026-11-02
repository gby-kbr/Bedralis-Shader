# Next Task

Last phase: 06 - Water system (merged to main; not compiled, not tested in game).
Last work: water shading in `water.h` / `wave.h` (fresnel, animated normals, sky/sun/moon reflection, rain ripples), wired into the `RenderChunk` fragment shader.

Unfinished:
- Cloud reflection on water (do it once clouds exist, Phase 07).
- Shadow integration and true N.L lighting (no shadow map or terrain normals).
- Thunder fog/lighting (needs thunder detection).
- Performance watch: `calculateEnvironment()` runs per vertex in `RenderChunk`; audit in Phase 11 (or move shared work out).
- Thunderstorm sky (needs thunder detection; `WeatherID` meaning not verified, do not guess).
- End sky (planned for Phase 10).
- Smooth underwater transition.

Files / functions in progress: none open. Phase 07 starts at `source/origin/function/clouds.h` and `noise.h` (both empty) and the `Clouds` material (currently pass-through with a distance fade).

Remaining errors: none known, but nothing has been compiled by the real material compiler yet.

Unmet dependencies:
- Material build tooling (`../../tools/data/materials` referenced by `project.json` is missing).
- In-game confirmation of: `DimensionID.x` values (0/1/2), `TimeOfDay` phase (code assumes 0 = noon), `FogAndDistanceControl` semantics used by `detectRain`/`detectUnderwater`, sky dome being camera-centered, and `a_color0.x` direction (0 zenith, 1 horizon), `MoonPhase.x` range (0 = full moon), and that `sunDir` matches the rendered sun (used to tell sun from moon in `SunMoon`); RenderChunk world position is absolute; `v_lightmapUV.y` is the sky light; `FogControl` (Actor) equals `FogAndDistanceControl`; `FogAndDistanceControl.json` does not duplicate a definition from the missing toolchain data; `v_lightmapUV` is (block, sky); the light tint does not over-darken the engine lightmap; `ViewPositionAndTime.w` is time in seconds; water is identified by vertex alpha < 0.95 in the TRANSPARENT path; the vec3 `v_ditheringAndMaskTinting` varying is accepted by the compiler.

Concrete next steps:
1. Phase 07 - Cloud system: cloud rendering, movement, lighting, weather integration and distance fade in `clouds.h` / `noise.h`; keep cloud positions stable in world space; then add cloud reflection to `shadeWater()`.
2. Re-check sunrise/sunset glow side, sun/moon tint, fog, light tint and water tuning once the shader can be tested in game.
