# Third-Party Licenses

RDNA Cast is built on top of and bundles software from the projects listed below. Each is distributed under its own license. RDNA Cast itself is dual-licensed: MIT for original additions ([installer_assets/LICENSE.txt](installer_assets/LICENSE.txt)) and GPL-2.0-or-later for the OBS Studio base ([COPYING](COPYING)).

This file is a courtesy summary — the authoritative license for each component is the LICENSE / COPYING file shipped in its respective source directory.

## Foundational

### OBS Studio
- **License**: GNU General Public License v2.0 (GPL-2.0-or-later)
- **Source**: https://github.com/obsproject/obs-studio
- **Copyright**: Copyright (C) Hugh Bailey ("Lain") and contributors — see [AUTHORS](AUTHORS)
- **License text**: [COPYING](COPYING)

## Bundled within `deps/`

### blake2
- **License**: CC0 1.0 Universal / Apache 2.0 / OpenSSL — see [deps/blake2/LICENSE.blake2](deps/blake2/LICENSE.blake2)
- **Source**: https://github.com/BLAKE2/BLAKE2

### json11
- **License**: MIT
- **Source**: https://github.com/dropbox/json11
- **License file**: [deps/json11/LICENSE.txt](deps/json11/LICENSE.txt)

### libcaption
- **License**: MIT
- **Source**: https://github.com/szatmary/libcaption
- **License file**: [deps/libcaption/LICENSE.txt](deps/libcaption/LICENSE.txt)

### glad
- **License**: MIT (with public-domain spec headers)
- **Source**: https://github.com/Dav1dde/glad

### libdshowcapture
- **License**: GPL-2.0-or-later
- **Source**: https://github.com/obsproject/libdshowcapture

### w32-pthreads
- **License**: LGPL-2.1
- **Source**: pthread-win32 / MinGW-w64 derivative

### media-playback
- **License**: GPL-2.0-or-later (OBS-internal)

## Linked at build time

### Qt 6
- **License**: LGPL-3.0 (and GPL-3.0 dual-licensed)
- **Source**: https://www.qt.io/

### FFmpeg
- **License**: LGPL-2.1 (default build) / GPL-2.0+ (with `--enable-gpl`)
- **Source**: https://ffmpeg.org/

### x264
- **License**: GPL-2.0-or-later
- **Source**: https://www.videolan.org/developers/x264.html

### libcurl
- **License**: curl license (MIT/X-style)
- **Source**: https://curl.se/

### zlib
- **License**: zlib license
- **Source**: https://zlib.net/

### libpng
- **License**: PNG Reference Library License v2 (libpng license)
- **Source**: http://www.libpng.org/

### mbedTLS / OpenSSL (TLS, depending on FFmpeg config)
- **License**: Apache-2.0 (mbedTLS) or Apache-2.0 / OpenSSL dual (OpenSSL)
- **Sources**: https://www.trustedfirmware.org/projects/mbed-tls/ , https://www.openssl.org/

### Detours
- **License**: MIT
- **Source**: https://github.com/microsoft/Detours

### Discord RPC (if linked)
- **License**: MIT

## SDKs (compiled against; no headers redistributed beyond what is required)

### AMD AMF SDK
- **License**: AMF SDK License (proprietary, redistribution permitted for compiled binaries)
- **Source**: https://github.com/GPUOpen-LibrariesAndSDKs/AMF
- **Copyright**: Copyright (c) Advanced Micro Devices, Inc. All rights reserved.

### Microsoft Media Foundation / Windows SDK headers
- License: Microsoft Windows SDK License Agreement

---

If you spot a missing or incorrectly-listed component, please open an issue at <https://github.com/georgekgr12/GK_OBS_LITE_AMD/issues>.
