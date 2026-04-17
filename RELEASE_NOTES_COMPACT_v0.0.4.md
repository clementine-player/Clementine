# 🎛️ Clementine Extended v0.0.4-experimental

**AI-Powered Stem Separation & Multitrack Mixing**

## ✨ What's New

🤖 **Offline AI Stem Separation** - Split any song into vocals, drums, bass, and other instruments using Demucs AI models  
🎚️ **Professional Mixing Interface** - Independent volume, mute, and solo controls for each stem  
🎵 **Quick Presets** - One-click karaoke mode, instrumental mode, and stem isolation  
⚡ **Zero-latency Playback** - GStreamer multitrack engine for seamless audio blending  
🔧 **Robust Processing** - 15-minute timeout protection with comprehensive error handling  

## 🚀 Key Features

- **Real-time Multitrack Mixing**: Control each stem independently during playback
- **High-Quality AI Separation**: Powered by state-of-the-art Demucs models
- **Automatic Model Management**: Downloads and manages 2-4GB AI models automatically  
- **Professional UI**: Dedicated AI Stem Mixer with volume sliders and mute toggles
- **GPL v3 Compliant**: Fully open-source with compatible dependencies

## 💾 Installation

```bash
# Download and extract release
# Set up Python environment  
python3 -m venv stem_separation_env
source stem_separation_env/bin/activate
pip install torch demucs

# Run application
./clementine
```

## 🎯 Quick Start

1. Open **View → AI Stem Mixer**
2. Load any audio file
3. Click **🤖 Separate Current Track** 
4. Wait for processing (2-5 minutes)
5. Use presets or manual controls to mix

## 📋 Requirements

- **OS**: Linux (Ubuntu 20.04+), Windows 10, macOS 10.15+  
- **RAM**: 8 GB (16 GB recommended)
- **Storage**: 5 GB free space
- **Python**: 3.8+ with pip support

## 🔗 Links

- [Full Release Notes](RELEASE_NOTES_v0.0.4-experimental.md)
- [License Information](LICENSE_COMPLIANCE.md)  
- [Source Code](https://github.com/your-repo/AIMP-Clementine)

---

**⚠️ Experimental Release**: Focus on stable offline separation. Live mode coming in v0.0.5.

**🎵 Transform Your Music Experience** - Download now and start mixing like a pro!
