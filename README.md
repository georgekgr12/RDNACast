<p align="center">
  <img src="installer_assets/rdnacast_logo.png" alt="RDNA Cast" width="280">
</p>

<h1 align="center">RDNA Cast</h1>

<p align="center">
  <strong>High-performance video recording &amp; streaming for AMD Radeon. Gaming unleashed.</strong><br>
  <em>~85% less RAM than stock OBS. Fixes AMD bitrate spikes. Built for gamers.</em>
</p>

<p align="center">
  <a href="https://github.com/karagioules/RDNA_Cast/releases/latest">Download</a> &bull;
  <a href="#features">Features</a> &bull;
  <a href="#performance">Performance</a> &bull;
  <a href="#amd-encoder-fixes">AMD Fixes</a> &bull;
  <a href="#building">Building</a>
</p>

## App Screenshots

<p align="center">
  <img src="docs/screenshots/rdna-cast-app-1.png" alt="RDNA Cast main window" width="32%">
  <img src="docs/screenshots/rdna-cast-app-2.png" alt="RDNA Cast tray controls" width="32%">
  <img src="docs/screenshots/rdna-cast-app-3.png" alt="RDNA Cast settings" width="32%">
  <br>
  <img src="docs/screenshots/status-overlay-settings.svg" alt="Status Overlay settings" width="68%">
</p>

---

## Disclaimer

**RDNA Cast is an independent open-source project.** It is **not affiliated with, endorsed by, or sponsored by** Advanced Micro Devices, Inc. or the OBS Project. "AMD", "Radeon", and "RDNA" are trademarks of Advanced Micro Devices, Inc., used here only descriptively to indicate target hardware compatibility.

RDNA Cast is a fork of [OBS Studio](https://obsproject.com), redistributed under the GNU General Public License v2.0. See [COPYING](COPYING) for the full GPL text and [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for bundled-library licenses.

## What is this?

RDNA Cast is a **lightweight fork of [OBS Studio](https://obsproject.com) 31.0.3**, built exclusively for **AMD hardware on Windows**. It strips out everything you don't need, optimizes what's left for AMD Zen 3+ CPUs and RDNA GPUs, and **fixes real AMD encoder bugs** that cause bitrate spikes and drops during streaming.

## Performance

| Metric | Stock OBS 31.0.3 | RDNA Cast |
|--------|-------------------|-----------|
| **RAM (idle)** | ~300-400 MB | **~52 MB** |
| **Threads** | ~50-60 | **~15** |
| **Plugin DLLs** | ~30+ | **12** |
| **Disk size** | ~500+ MB | **139 MB** (installer) |
| **Preview GPU load** | Full refresh rate (165 FPS) | **~40 FPS** (throttled) |
| **Gaming FPS impact** | 20-30 FPS loss reported | **Significantly reduced** |

## Features

### Core
- **AMD-first encoder defaults** — AMF HEVC is the default encoder. Dropdown: AMD AV1 > HEVC > H.264 > x264 fallback
- **11 plugins** instead of 36 — only what you need for streaming and recording
- **AVX2 + /favor:AMD64 compiler optimizations** — built for Zen 3+ instruction scheduling
- **D3D11-only rendering** — AMD's best-optimized driver path on Windows

### Gaming-Friendly
- **Preview throttled to ~40 FPS** — GPU renders preview every 4th frame instead of 165 FPS. Encoder still gets every frame at full quality
- **Pre-analysis disabled** — eliminates the AMF pre-encode pass that steals GPU cycles from your game
- **Reduced GPU encode textures** (10 → 6) — frees VRAM for game textures
- **Reduced async frame cache** (30 → 12) — less memory pressure
- **No profiler overhead** in the render loop

### UX
- **Portable by default** — settings, scenes, profiles live next to the executable
- **Close-to-tray** — X minimizes to system tray (like Steam/AMD Adrenalin). Recording keeps running
- **Tray controls** — right-click for Stream, Record, Replay Buffer, Save Replay, Exit
- **Performance status HUD** — optional always-on-top opaque badge with corner placement
- **Combined output status** — shows `LIVE`, `REC`, `REPLAY`, or combined states like `LIVE + REC + REPLAY`
- **Capture-hidden overlay window** — the overlay is marked to stay out of supported Windows captures while remaining visible on your monitor
- **Built-in assisted updater** — Help → Check for Updates fetches the latest installer from GitHub Releases
- **About dialog** with GPLv2-or-later, MIT additions, and bundled-library notices
- **Windows installer** with custom branding via Inno Setup

### Status Overlay

The optional status overlay lives in **Settings → General → Status Overlay**. It is now a persistent performance HUD designed to avoid the stutter risk of translucent overlays:

- Four fixed screen positions: top-left, top-right, bottom-left, bottom-right
- Opaque, tiny badge with no window opacity or translucent background
- Hidden while idle
- Visible only when streaming, recording, or replay buffer is active
- Combines active states, for example `REC + REPLAY` or `LIVE + REC + REPLAY`
- Briefly flashes `SAVE` when saving replay while the replay buffer is active
- Uses Windows display affinity to keep the overlay out of captures where supported

### AMD Encoder Fixes

These fixes address long-standing AMD AMF issues reported across [OBS #12013](https://github.com/obsproject/obs-studio/issues/12013), [OBS #12512](https://github.com/obsproject/obs-studio/issues/12512), [AMF #465](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/465), and [AMF #323](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/323):

| Fix | Problem | Solution |
|-----|---------|----------|
| **AV1 filler data bug** | Stock OBS compares AV1 rate control against H.264 enum constants — filler data never activates for AV1 CBR. Causes bitrate drops to 0 on static scenes. | Use correct `AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_CBR` enum |
| **VBV buffer too large** | Stock OBS sets VBV = 1 full second of bitrate, allowing spikes up to ~2x target before correction | VBV = 500ms (`bitrate / 2`) — cuts maximum spike in half |
| **Startup encoder overload** | H.264 pre-analysis (`PREENCODE_ENABLE`) consumes first GOP, causing 2-3 second frame drops at stream start | Pre-analysis disabled |
| **HQCBR filler data** | High Quality CBR mode didn't enable filler data on any codec | Filler data now enabled for both CBR and HQCBR on H.264, HEVC, and AV1 |

## Target Hardware

| Component | Tested On |
|-----------|-----------|
| **CPU** | AMD Ryzen 7 5700X3D (Zen 3, 8C/16T) |
| **GPU** | AMD Radeon RX 9070 XT (RDNA 4) |
| **RAM** | 32GB DDR4 |
| **OS** | Windows 11 Pro |

Any AMD system with a Zen 2+ CPU and RDNA+ GPU should work. AVX2 required (all Zen CPUs support this).

## What's Removed

### Non-AMD Encoders
- `obs-nvenc` — NVIDIA GPU encoder
- `obs-qsv11` — Intel Quick Sync Video
- `nv-filters` — NVIDIA GPU filters

### Non-Windows Platform Code
- `libobs-opengl`, `libobs-metal`, all macOS/Linux/BSD plugins

### Heavyweight Optional Plugins
- `obs-browser` — Chromium (can be re-enabled with `-DENABLE_BROWSER=ON`)
- `obs-websocket`, `obs-webrtc`, `vlc-video`, `decklink`, `aja`, `obs-vst`, `text-freetype2`, `obs-libfdk`
- Lua/Python scripting, frontend-tools plugin

### What Remains (11 core plugins)

| Plugin | Purpose |
|--------|---------|
| `obs-ffmpeg` | FFmpeg muxing + **AMF hardware encoder** (H.264, HEVC, AV1) |
| `obs-x264` | Software H.264 fallback |
| `obs-filters` | Video/audio filters |
| `obs-transitions` | Scene transitions |
| `obs-outputs` | RTMP, SRT, RIST streaming + recording |
| `obs-text` | Windows text rendering |
| `image-source` | Image/slideshow sources |
| `rtmp-services` | Streaming service configs (Twitch, YouTube, etc.) |
| `win-capture` | Window, game, and monitor capture |
| `win-dshow` | Webcams and capture cards (DirectShow) |
| `win-wasapi` | Windows audio capture/output |

## Building

### Prerequisites
- Visual Studio 2022 (Community or BuildTools) with **Desktop development with C++** workload + ATL
- CMake 3.28+
- Git
- Inno Setup 6 (for installer, optional)

### Quick Build
```cmd
git clone https://github.com/karagioules/RDNA_Cast.git
cd RDNA_Cast
cmake --preset amd-lite-x64
cmake --build build_amd_lite --config RelWithDebInfo --parallel
```

### With Browser Source
```cmd
cmake --preset amd-lite-x64 -DENABLE_BROWSER=ON -DENABLE_WEBSOCKET=ON
cmake --build build_amd_lite --config RelWithDebInfo --parallel
```

### Full Build + Installer
Run `build.bat` — handles CMake configure, build, and Inno Setup packaging in one step.

## How It Works

All RDNA Cast changes are gated behind the `OBS_AMD_LITE` CMake flag and `#ifdef OBS_AMD_LITE` preprocessor guards. Building without the flag produces stock OBS Studio. This makes upstream merges from [obsproject/obs-studio](https://github.com/obsproject/obs-studio) straightforward.

## Credits

- [OBS Studio](https://obsproject.com) by the OBS Project — the foundation this project is built on
- [AMD AMF SDK](https://github.com/GPUOpen-LibrariesAndSDKs/AMF) — hardware encoding interface
- All upstream OBS contributors listed in [AUTHORS](AUTHORS)

If you use RDNA Cast and want to support the upstream project that makes it possible, please **[contribute to OBS Studio](https://obsproject.com/contribute)**.

## License

RDNA Cast is an OBS Studio fork. The combined application and binary distributions are redistributed under the **GNU General Public License v2.0 or later**; see [COPYING](COPYING).

Original RDNA Cast additions are also offered under the **MIT License** by George Karagioules. That MIT grant applies only to original RDNA Cast additions and does not relicense OBS Studio or the combined OBS-derived application.

Bundled third-party libraries retain their own licenses. See [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md). Installers ship `LICENSE.txt`, `COPYING`, `AUTHORS`, and `THIRD_PARTY_LICENSES.md` alongside the application.
