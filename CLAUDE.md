# RDNA Cast — Project Context

(Internal flag is still `OBS_AMD_LITE`, source layout is unchanged. This is the same project, rebranded from "GK OBS Lite AMD Edition" in v0.7.0 for trademark compliance.)

## Target Hardware
- **CPU**: AMD Ryzen 7 5700X3D (Zen 3, 8C/16T, 3D V-Cache)
- **GPU**: AMD Radeon RX 9070 XT (RDNA 4)
- **RAM**: 32GB DDR4
- **OS**: Windows 11 Pro
- **MSVC flags**: `/arch:AVX2 /favor:AMD64`

## Build
```cmd
cmake --preset amd-lite-x64
cmake --build build_amd_lite --config RelWithDebInfo --parallel
```

## Repo
- GitHub: https://github.com/georgekgr12/RDNACast (renamed from `GK_OBS_LITE_AMD` in v0.7.0)
- Releases: https://github.com/georgekgr12/RDNACast/releases
- Branch: `main` only (no dev branch)
- Tags: semver starting at `v0.5.0`
- Based on OBS Studio 31.0.3 (upstream remote: `upstream`)

## Architecture
- All AMD Lite changes gated behind `OBS_AMD_LITE` CMake flag and `#ifdef OBS_AMD_LITE` preprocessor guards
- Stock OBS build path is preserved when flag is OFF
- D3D11 is the sole graphics backend
- AMF encoder uses D3D11 texture sharing for zero-copy GPU encoding
- UI directory is `UI/` (OBS 31.x layout), not `frontend/` (32.x layout)

## Key Files Modified (from stock OBS 31.0.3)

### Build System
- `CMakeLists.txt` — `OBS_AMD_LITE` option, strip OpenGL/tests, scripting OFF
- `CMakePresets.json` — `amd-lite-x64` preset
- `plugins/CMakeLists.txt` — 11-plugin AMD-only build, `ENABLE_BROWSER`/`ENABLE_WEBSOCKET` options
- `cmake/windows/compilerconfig.cmake` — AVX2, /favor:AMD64, `OBS_AMD_LITE` define

### Portable Mode
- `UI/obs-app.cpp` — `portable_mode = true`, `opt_disable_updater = true`, tray defaults

### Encoder Defaults
- `UI/window-basic-main.cpp` — `InitBasicConfigDefaults2()` → AMD HEVC default for new profiles
- `UI/window-basic-auto-config.cpp` — `preferHardware` includes AMD, `GetEncoderId()` → AMD HEVC
- `UI/window-basic-auto-config-test.cpp` — Priority: AMD > NVENC > QSV > x264
- `UI/window-basic-settings-stream.cpp` — Dropdown: AMD AV1 > HEVC > H.264 > x264; fallback → AMD

### Close-to-Tray
- `UI/window-basic-main.cpp` — `closeEvent()` hides to tray, tray menu with Save Replay, `ForceClose()`
- `UI/window-basic-main.hpp` — `forceClose`, `sysTrayCloseToTray()`, `ForceClose()`, `sysTrayReplayBufferSave`

### Branding
- `UI/data/locale/en-US.ini` — `Basic.SystemTray.ClosedToTray` string
- `UI/forms/images/obs.png`, `tray_active.png`, `obs_paused.png` — custom red/black logo
- `cmake/bundle/windows/obs-studio.ico`, `UI/cmake/windows/obs-studio.ico` — custom .ico

### Extra Files
- `build.bat` — one-click build + Inno Setup packaging
- `installer.iss` — Inno Setup installer script
- `installer_assets/portable_mode` — sentinel file for portable installs
- `Logo.png`, `Logo-removebg-preview.png` — source logo files

## Plugins Included (11)
obs-ffmpeg, obs-x264, obs-filters, obs-transitions, obs-outputs, obs-text, image-source, rtmp-services, win-capture, win-dshow, win-wasapi

## Plugins Removed
obs-nvenc, obs-qsv11, nv-filters, obs-browser, obs-websocket, obs-webrtc, vlc-video, decklink, aja, obs-vst, text-freetype2, obs-libfdk, coreaudio-encoder, all macOS plugins, all Linux/BSD plugins, oss-audio, sndio

## User Preferences
- User: Georg (georgekgr12 on GitHub)
- Publisher name in installers: George Karagioules
- Inno Setup installed at: `%LOCALAPPDATA%\Programs\Inno Setup 6\`
- Has VS2022 BuildTools + CMake 4.3 installed
- Prefers direct action over explanation
- Wants single-file portable executables when possible
- Uses `H:\DevWork\Win_Apps\` as dev workspace
