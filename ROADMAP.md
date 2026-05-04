# RDNA Cast — Feature Roadmap

## Tier 1 — High Value, Feasible

### 1. Auto-Detect AMD GPU & Set Optimal Defaults
On first launch, detect the exact GPU model (RX 7600, 7900 XTX, 9070 XT, etc.) via DXGI adapter enumeration and auto-configure the best encoder settings:

| GPU Generation | Default Encoder | CQP | Preset | Notes |
|---------------|----------------|-----|--------|-------|
| RDNA 4 (RX 9000) | AV1 | 18 | speed | VCN 5.0 — excellent AV1 quality |
| RDNA 3 (RX 7000) | HEVC | 20 | speed | VCN 4.0 — good HEVC, AV1 available but less efficient |
| RDNA 2 (RX 6000) | H.264 | 20 | balanced | VCN 3.0 — AV1 not available, HEVC OK |
| RDNA 1 (RX 5000) | H.264 | 22 | speed | VCN 2.0 — H.264 only reliable codec |
| Vega / Polaris | H.264 | 23 | speed | VCE 3/4 — warn about limited quality |

**Implementation:** Read `DXGI_ADAPTER_DESC.DeviceId` on startup, map to RDNA generation, set `SimpleOutput/StreamEncoder` and `SimpleOutput/RecEncoder` accordingly. Show a one-time "Configured for your [GPU name]" notification.

**Files:** `UI/window-basic-main.cpp` (InitBasicConfigDefaults2), `plugins/obs-ffmpeg/obs-amf-test/obs-amf-test.cpp` (GPU detection reference)

---

### 2. AMD GPU Stats in Status Bar
Display real-time AMD GPU metrics directly in the OBS status bar:
- GPU Usage %
- GPU Temperature (°C)
- VRAM Usage (used / total)
- VCN Encoder Load %

**Data sources:**
- WMI: `Win32_VideoController` for basic info
- ADLX SDK (AMD Display Library): Full GPU telemetry (temp, clock, usage, VRAM)
- Performance counters: `D3DKMT` queries for GPU engine utilization

**UI:** Add a small GPU stats section to the right side of the status bar, next to the existing CPU% and FPS counters. Format: `GPU: 45% 62°C | VRAM: 4.2/16GB`

**Files:** `UI/window-basic-status-bar.cpp` (new timer + display), new `UI/amd-gpu-stats.cpp/.hpp` for the monitoring backend

---

### 3. AMD-Tuned Encoding Presets
Replace the generic quality presets with AMD-specific workflow presets in the Simple Output settings:

| Preset Name | Use Case | Encoder | Rate Control | Bitrate/CQP | VBV | B-Frames | Notes |
|------------|----------|---------|-------------|-------------|-----|----------|-------|
| **Gaming (Background)** | Replay buffer while gaming | HEVC | CQP 22 | — | — | 0 | Minimal GPU impact |
| **Streaming (Twitch)** | Live streaming at 1080p60 | HEVC | CBR | 6000 kbps | 3000 | 0 | Filler data ON, tight VBV |
| **Streaming (YouTube)** | Live streaming at 1440p60 | AV1 | CBR | 12000 kbps | 6000 | 0 | AV1 preferred for YT |
| **Cinematic Recording** | High quality local recording | AV1 | CQP 16 | — | — | 2 | 10-bit if display supports |
| **Clip It** | Short replay clips (30-120s) | HEVC | CQP 20 | — | — | 0 | Optimized for fast saves |
| **Balanced** | General purpose | HEVC | CQP 20 | — | — | 0 | Good quality, low overhead |

**Implementation:** Add a "Preset" dropdown above the encoder selection in Simple Output settings. Selecting a preset auto-fills encoder, rate control, bitrate, and quality settings. User can still manually override.

**Files:** `UI/window-basic-settings.cpp` (new preset dropdown), `UI/window-basic-main-outputs.cpp` (preset application logic)

---

### 4. Smart Encoder Fallback
If the AMD hardware encoder fails to initialize (driver crash, GPU busy, AMF error), automatically retry with x264 software encoding instead of showing an error and stopping.

**Current behavior:** OBS shows "Failed to start recording/streaming" error dialog.
**New behavior:**
1. AMF encoder init fails → log warning
2. Silently create x264 encoder with equivalent settings (map CQP to CRF, map bitrate)
3. Show tray notification: "AMD encoder unavailable — using software fallback"
4. When streaming/recording stops, retry AMF next time

**Files:** `UI/window-basic-main-outputs.cpp` (output handler), `plugins/obs-ffmpeg/texture-amf.cpp` (error detection)

---

## Tier 2 — Medium Value, More Work

### 5. AMD FidelityFX Super Resolution (FSR) for Preview
Use AMD's FSR algorithm to upscale a lower-resolution preview render, saving significant GPU bandwidth.

**How it works:**
- Render the preview scene at 50% resolution (e.g., 960x540 instead of 1920x1080)
- Apply FSR upscaling shader to display at full preview window size
- Encoder still receives full-resolution frames — output quality unchanged

**Expected savings:** ~60-75% less GPU work for preview rendering (on top of our existing frame throttle).

**Implementation:** Add FSR 1.0 shader (open source, MIT licensed from GPUOpen) as a post-process step in `render_display()`. Toggle via Settings → Advanced → "Use FSR for preview scaling".

**Files:** `libobs/obs-display.c` (render pipeline), new `libobs/graphics/fsr.effect` shader file
**Reference:** https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-FSR

---

### 6. GPU Load-Aware Quality Throttling
Monitor GPU utilization in real-time during recording/streaming. If GPU load exceeds a threshold, temporarily reduce encoding quality to prevent frame drops.

**Algorithm:**
```
every 2 seconds:
  if GPU_usage > 95% for 6+ seconds:
    increase CQP by 4 (e.g., 18 → 22)
    log: "Reducing recording quality to prevent frame drops"
    show tray notification
  if GPU_usage < 80% for 10+ seconds AND quality was reduced:
    restore original CQP
    log: "Restoring recording quality"
```

**Implementation:** New background thread polling GPU usage via ADLX/WMI, dynamically calling `obs_encoder_update()` to adjust CQP.

**Files:** New `UI/amd-gpu-throttle.cpp/.hpp`, `UI/window-basic-main-outputs.cpp` (encoder update hooks)

---

### 7. One-Click Instant Replay Setup
A single button in the main UI that configures everything for instant replay:

**What it does (one click):**
1. Enables replay buffer
2. Sets duration to 60 seconds (configurable)
3. Sets encoder to AMD HEVC, CQP 20, speed preset
4. Sets output format to Hybrid MP4
5. Sets save path to user's Videos folder
6. Assigns hotkey Ctrl+Shift+R for "Save Replay"
7. Starts the replay buffer immediately
8. Shows confirmation: "Instant Replay enabled — press Ctrl+Shift+R to save last 60 seconds"

**UI:** Add an "Instant Replay" toggle button in the Controls dock, next to Start Recording.

**Files:** `UI/window-basic-main.cpp` (new slot), `UI/window-basic-main-outputs.cpp` (replay buffer config)

---

## Tier 3 — Nice to Have

### 8. AMD Color Space / HDR Optimization
Auto-detect HDR display and configure the encoding pipeline for HDR passthrough:

- Detect Windows HDR status via `DXGI_OUTPUT_DESC1.ColorSpace`
- If HDR active: set video format to P010 (10-bit), set color space to Rec. 2100 PQ
- Configure AMF encoder for 10-bit HEVC/AV1 with HDR metadata
- Show notification: "HDR mode detected — recording in 10-bit HDR"

**Files:** `libobs-d3d11/d3d11-subsystem.cpp` (HDR detection), `UI/window-basic-main.cpp` (auto-config), `plugins/obs-ffmpeg/texture-amf.cpp` (10-bit settings)

---

### 9. Freesync-Aware Preview
Prevent OBS preview rendering from interfering with AMD Freesync/VRR:

- Query the display's Freesync range via ADLX
- Cap OBS preview FPS to stay within the Freesync range floor (e.g., 48 FPS minimum)
- Avoid irregular frame pacing that could cause Freesync to disengage
- When a game is fullscreen, reduce preview to minimum viable FPS (10-15)

**Files:** `libobs/obs-video.c` (frame throttle logic), new `UI/amd-freesync.cpp` for VRR detection

---

## Priority Order (Recommended Implementation Sequence)

1. **Auto-Detect GPU** (#1) — biggest UX win, low effort
2. **AMD Presets** (#3) — high value for new users
3. **One-Click Replay** (#7) — killer feature for gamers
4. **Smart Fallback** (#4) — reliability improvement
5. **GPU Stats** (#2) — quality of life
6. **FSR Preview** (#5) — performance win
7. **GPU Throttling** (#6) — advanced optimization
8. **HDR** (#8) — niche but valuable
9. **Freesync** (#9) — polish

---

## Technical Notes

- All features gated behind `#ifdef OBS_AMD_LITE`
- AMD GPU detection uses `DXGI_ADAPTER_DESC.VendorId == 0x1002`
- RDNA generation determined by Device ID ranges (documented in AMD's public PCI ID database)
- ADLX SDK is optional — features degrade gracefully if not available
- No AMD driver dependency beyond what AMF already requires
