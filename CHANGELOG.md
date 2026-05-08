# Changelog

All notable changes to this project. Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.7.0] — 2026-05-04

### Changed — Rebrand
- **Project rebranded from `GK OBS Lite AMD Edition` to `RDNA Cast`** for trademark compliance
- New About dialog text, window title, and installer branding
- New product name in all user-facing strings (en-US.ini)
- Added explicit non-affiliation disclaimer to README, About dialog, and installer license notice — clarifies independence from AMD and the OBS Project
- Added `THIRD_PARTY_LICENSES.md` aggregating all bundled-library licenses
- Added this `CHANGELOG.md`

### Migrated
- App config directory renamed from `%APPDATA%\obs-studio` to `%APPDATA%\rdnacast`. Existing v0.6.x users get a one-time auto-migration on first launch (the old folder is renamed; settings, scenes, profiles, hotkeys are preserved). Same for portable installs.
- Inno Setup AppId is unchanged so existing v0.6.x users get a clean in-place upgrade. Old `GK_OBS_Lite_AMD` shortcuts are removed by the installer.

## [0.6.7] — 2026-05-04

### Removed
- Stripped upstream OBS Help menu entries: Help Portal, Website, Discord, What's New, Release Notes, Check for Updates (the latter was already disabled, just hidden from menu now). Help menu now: Log Files, Crash Logs, Repair, Restart Safe, About.

### Fixed
- `build.bat`: `vswhere` now uses `-products *` to detect VS2022 BuildTools (was only matching Community/Pro)
- `cmake/common/versionconfig.cmake`: strip leading `v` from git tags before extracting `MAJOR.MINOR.PATCH` for `project(VERSION ...)`

## [0.6.6] — 2026-03-28

### Added
- **Configurable preview FPS setting** — Preview dropdown with 5 options: Full (60), Smooth (30, default), Balanced (20), Efficient (15), Minimal (10). Lower preview FPS frees CPU/GPU for the game.

## [0.6.5] — 2026-03-28

### Added
- Tray notifications for hotkey-triggered actions

## [0.6.4] — 2026-03-28

### Fixed
- Settings dialog crash
- YouTube chat dock position not persisting

## [0.6.3] — 2026-03-28

### Changed
- Optimized streaming panel rendering
- Improved preview FPS throttling

## [0.6.2] — 2026-03-28

### Added
- YouTube Connect dock for stream management
- Browser sources (build with `-DENABLE_BROWSER=ON`)
- Persistent UI/dock settings across restarts

## [0.6.0] — earlier 2026

### Added
- Custom assisted-update system pulling from GitHub Releases
- Tray controls (Stream, Record, Replay Buffer, Save Replay, Exit)
- About dialog with GPLv2-or-later, MIT additions, and bundled-library attribution

## [0.5.0] — initial public release

### Added
- AMD-only build (AMF HEVC default encoder)
- AMD AMF encoder fixes (AV1 filler data, VBV cap, pre-analysis disabled, HQCBR filler data)
- Stripped non-AMD plugins (NVENC, QSV, browser, websocket, etc.)
- Portable mode by default
- Close-to-tray behavior
- Inno Setup installer with custom branding

[0.7.0]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.7.0
[0.6.7]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.6.7
[0.6.6]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.6.6
[0.6.5]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.6.5
[0.6.4]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.6.4
[0.6.3]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.6.3
[0.6.2]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.6.2
[0.5.0]: https://github.com/georgekgr12/RDNACast/releases/tag/v0.5.0
