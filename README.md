# Bedralis-Shader

Realistic shader for Minecraft Bedrock (RenderDragon).

## Build

Requirements: Python 3.11+ and `pip install -r requirements.txt`.

```
build.bat setup                        (Windows)
build.bat pack -p multiplatform
./build.sh setup                       (Linux / Android, Termux)
./build.sh pack -p android
```

- `setup` downloads `shaderc` and the base material definitions (from the Newb Shader `dev` release).
- `pack -p <profile>` builds the pack into `build/pack-<profile>`; profiles: `android`, `ios`, `windows`, `multiplatform`. Direct3D (Windows) needs Windows to compile.
- `--no-zip` skips the `.mcpack` archive.
- `mats -p <profile> -m Sky` builds single materials.
- The pack manifest (name, version, UUIDs) lives only in `assets/manifest.json` and is copied unchanged into the built pack. The material list and credits live in `source/pack_config.toml`.

GitHub Actions: `build` (manual, multi-platform pack artifact) and `build-check` (runs on pull requests and pushes to `main` and fails when the pack is not ready to build).
