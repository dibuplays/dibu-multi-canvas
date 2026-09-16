# Dibu Multi-Canvas Studio

An early native OBS Studio plugin for managing an additional canvas and linking normal OBS scenes to canvas-specific scenes.

## Normal user installation

Release builds produce `Dibu-Multi-Canvas-Studio-0.2.0-windows-x64-Installer.exe`. Users only need to:

1. Close OBS Studio.
2. Double-click the installer.
3. Confirm the detected OBS folder.
4. Finish installation and reopen OBS.

The installer validates that OBS 31.1 or newer is installed and places the DLL and plugin data in the correct OBS folders automatically. Users do **not** need CMake, Visual Studio, the OBS SDK, or the source package.

The Windows installer is produced by the included GitHub Actions workflow. Until that Windows build completes successfully, the repository contains source and installer infrastructure—not a verified end-user binary.

## Current MVP

- Creates or reuses a `Dibu Multi-Canvas - Vertical` canvas.
- Configures an independent, even-sized video mix (default `1080 × 1920`).
- Creates canvas-owned scenes.
- Links each normal OBS scene to a canvas scene.
- Switches the canvas scene whenever the main OBS scene changes.
- Saves resolution, enabled state, and scene links.
- Provides an OBS dock for canvas controls.

This is source code for an MVP, not a finished Aitum replacement. Streaming, recording, preview rendering, source editing, audio routing, installers, and smart layout conversion are intentionally listed as later milestones.

## Requirements

- OBS Studio 31.1 or newer (the project uses the native canvas API).
- OBS development files (`libobs` and `obs-frontend-api`).
- Qt 6 Widgets/Core development files.
- CMake 3.22 or newer.
- A C++17 compiler.

## Validate the dependency-free core

```bash
cmake --preset core-tests
cmake --build --preset core-tests
ctest --preset core-tests
```

## Build the plugin on Windows

Install or build the OBS SDK/dependencies, then make their CMake packages discoverable through `CMAKE_PREFIX_PATH`.

```powershell
cmake --preset windows-x64 -DCMAKE_PREFIX_PATH="C:\path\to\obs-deps;C:\path\to\obs-build"
cmake --build --preset windows-x64
```

The included release workflow builds the plugin, arranges the standard OBS directory structure and compiles a modern Inno Setup `.exe` installer. Pushing a semantic version tag such as `0.2.0` also creates a draft GitHub release with the installer, portable ZIP and checksums.

## Architecture

- `CanvasService`: owns the OBS canvas lifecycle and canvas scenes.
- `DockWidget`: controls resolution and linked scenes.
- `SettingsStore`: persists plugin settings safely with backup files.
- `LinkModel`: dependency-free scene-linking logic with tests.

## Next milestones

1. Render the canvas into a native OBS preview dock.
2. Add canvas-specific Sources and Scenes docks.
3. Add independent recording and replay/backtrack.
4. Add independent streaming output and audio routing.
5. Add Smart Layout Conversion presets: Fill, Fit, Corner, Stack, Safe Zone and Manual Lock.
6. Package signed Windows installer builds and test against supported OBS releases.

## Originality and licensing

This implementation was written as a clean MVP around public OBS APIs. It does not include Aitum branding or copied Aitum implementation files.

License: GPL-2.0-or-later. See `LICENSE`.
