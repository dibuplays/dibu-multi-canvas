# Roadmap

## 0.1 — Canvas foundation

- [x] Native OBS module scaffold
- [x] Additional canvas lifecycle
- [x] Configurable vertical resolution
- [x] Canvas-owned scene creation
- [x] Linked scene switching
- [x] Persistent settings
- [x] Core model tests

## 0.2 — Production UI

- [x] Automated Windows build infrastructure
- [x] One-click Inno Setup installer definition
- [x] OBS installation detection and minimum-version check
- [ ] Live preview using `obs_display_t`
- [ ] Canvas Scenes dock
- [ ] Canvas Sources dock
- [ ] Edit Transform support
- [ ] Add/remove/reorder sources
- [ ] Scene collection lifecycle hardening

## 0.3 — Output

- [ ] Independent recording output
- [ ] Replay/backtrack buffer
- [ ] Independent RTMP/WHIP streaming
- [ ] Encoder settings
- [ ] Audio track routing

## 0.4 — Dibu Smart Layout

- [ ] Import sources from a main scene
- [ ] Fill/Fit/Corner/Stack layout policies
- [ ] Safe-zone editor
- [ ] Crop-aware horizontal-to-vertical conversion
- [ ] Per-source manual lock
- [ ] Layout templates for gameplay, chatting and podcast scenes

## Release gate

Do not describe the project as an Aitum replacement until preview, source editing, streaming, recording, audio routing, crash recovery, and installers have been tested on supported OBS versions.
