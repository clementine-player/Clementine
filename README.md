Clementine Extended ![all](https://github.com/clementine-player/Clementine/workflows/all/badge.svg)
----------------

Clementine Extended is a modern music player and library organizer with AI-powered stem separation capabilities for Windows, Linux and macOS.

### 🎛️ AI Stem Separation Features
- **Real-time Audio Separation**: Split songs into individual stems (vocals, drums, bass, other)
- **Interactive Mixing**: Control volume, mute, and solo individual stems during playback
- **Offline Processing**: Uses Demucs AI models for high-quality separation
- **Quick Presets**: One-click karaoke, instrumental, and single-stem modes
- **GStreamer Integration**: Seamless multitrack playback with zero-latency mixing

- [Latest Release](https://github.com/clementine-player/Clementine/releases/latest)
- [Latest Pre-Releases](https://github.com/clementine-player/Clementine/releases)
- Website: http://www.clementine-player.org/
- Github: https://github.com/clementine-player/Clementine

System Requirements
----------------

### Minimum Requirements

- **CPU:** Dual-core processor, 2.0 GHz or better (AI separation requires more processing power)
- **RAM:** 4 GB (8 GB recommended for AI stem separation)
- **Disk Space:** 500 MB for installation + 2-4 GB for AI models
- **Display:** 1024x768 resolution
- **Sound Card:** Any compatible sound card
- **Network:** Internet connection for downloading AI models and online features
- **Python Environment:** Python 3.8+ with pip (for AI stem separation)

### Platform-Specific Requirements

#### Windows
- Windows 7 SP1 or later (64-bit)
- DirectX 9.0c or later
- Microsoft Visual C++ Redistributable 2019

#### Linux
- Linux kernel 3.10 or later
- GStreamer 1.14 or later with multitrack support
- Qt 5.12 or later
- Modern desktop environment (GNOME, KDE, etc.)
- Python 3.8+ with pip and venv
- PyTorch and Demucs libraries (auto-installed)

#### Raspberry Pi Support
- **Minimum**: Raspberry Pi 5 (8GB) - **Recommended for AI stem separation**
- **Alternative**: Raspberry Pi 4 (8GB) - Limited AI performance, basic playback only
- **OS**: Raspberry Pi OS (64-bit) Bookworm or newer
- **RAM**: 8GB minimum (AI stem separation requires 4-6GB RAM)
- **Storage**: 32GB+ SD Card (Class 10 or better) + USB 3.0 SSD recommended
- **Audio**: Hardware audio output enabled (HDMI/3.5mm/USB DAC)
- **Performance Note**: AI stem separation is CPU-intensive and may take 5-10x longer than x86_64 systems

Opening an issue
----------------
### Ask for a new feature

Please:

 * Check if the new feature is not already implemented (Changelog)
 * Check if another person didn't already open an issue
 * If there is already an opened issue there is no need to comment "+1", it won't help. Instead, you can subscribe to the issue to be notified of anything new about it

### Report a bug

Please:
 
 * Try the latest build (https://github.com/clementine-player/Clementine/releases) to see if any bug is still present. If it works fine even though you see an open issue, please comment on it and explain that the issue has been fixed.
 * Check if another person has already opened the same issue to avoid duplicates
 * If there already is an open issue you could comment on it to add detail about the problem or confirm it
 * In case there isn't, you can open a new issue with an explicit title and as much information as possible (OS, Clementine version, how to reproduce the problem...)
 * Please use https://pastebin.com/ for logs/debug.
 
If there are no answers, it doesn't mean we don't care about your feature request/bug. It just means we can't reproduce the bug or haven't had time to implement it :o)

Compiling from source
---------------------

Get the code (if you haven't already):

    git clone https://github.com/clementine-player/Clementine.git && cd Clementine

Setup Python environment for AI features:

    python3 -m venv stem_separation_env
    source stem_separation_env/bin/activate
    pip install torch demucs

Compile and install:

    mkdir build && cd build
    cmake ..
    make -j$(nproc)
    sudo make install

### First-time AI Setup
After installation, enable stem separation in the UI:
1. Open the AI Stem Mixer (View → AI Stem Mixer)
2. Load a track and click "🤖 Separate Current Track" 
3. Demucs models will be automatically downloaded (~2-4 GB)
4. Use preset buttons or manual controls for real-time mixing

See the Wiki for more instructions and a list of dependencies:
https://github.com/clementine-player/Clementine/wiki#compiling-and-installing-clementine

License
-------

Clementine Extended is licensed under the **GNU General Public License v3.0** (GPL v3).

This means:
- ✅ You can freely use, modify, and distribute this software
- ✅ You can use it for commercial purposes  
- ✅ You must make source code available when distributing
- ✅ You must preserve copyright and license notices
- ✅ Any derivative works must also be GPL v3 licensed

### Dependencies
All included AI libraries are GPL-compatible:
- **PyTorch**: BSD 3-Clause License (compatible)
- **Demucs**: MIT License (compatible)  
- **ONNX Runtime**: MIT License (compatible)
- **TensorFlow Lite**: Apache 2.0 License (compatible)

For complete license information, see:
- [COPYING](COPYING) - Full GPL v3 license text
- [LICENSE_COMPLIANCE.md](LICENSE_COMPLIANCE.md) - Detailed dependency analysis

© 2025 AI Stem Separation Implementation. All rights reserved under GPL v3.
