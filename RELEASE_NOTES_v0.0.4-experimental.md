# Clementine Extended v0.0.4-experimental

**Release Date**: September 6, 2025  
**License**: GNU GPL v3 (fully compliant)

## 🎛️ Major Features

### AI Stem Separation & Multitrack Mixing
- **Offline Stem Separation**: High-quality audio separation using Demucs AI models
- **Real-time Multitrack Playback**: Independent volume, mute, and solo controls for each stem
- **Interactive Mixing Interface**: Dedicated AI Stem Mixer widget with professional controls
- **Quick Presets**: One-click karaoke mode, instrumental mode, and single-stem isolation
- **Zero-latency Mixing**: GStreamer-powered multitrack engine for seamless audio blending

### Supported Stem Types
- 🎤 **Vocals**: Lead and backing vocals
- 🥁 **Drums**: Percussion and rhythmic elements  
- 🎸 **Bass**: Bass guitar and low-frequency instruments
- 🎹 **Other**: Piano, guitar, and remaining instruments

## 🔧 Technical Improvements

### Robust Processing Pipeline
- **15-minute timeout protection** with comprehensive diagnostics
- **Poll-based progress parsing** for reliable status updates
- **Error surface integration** displaying Python errors in the UI
- **Automatic model management** with 2-4GB Demucs model downloads
- **Memory optimization** for large audio files

### Enhanced User Experience  
- **Progress indicators** with real-time percentage updates
- **Offline-first approach** focusing on stable, high-quality separation
- **Professional UI controls** with volume sliders and mute toggles
- **State persistence** remembering user preferences across sessions
- **Integrated help system** with preset explanations

## 📋 System Requirements

### Minimum System Requirements
- **OS**: Linux (Ubuntu 20.04+), Windows 10, macOS 10.15+
- **CPU**: Dual-core 2.0 GHz (Quad-core recommended for AI processing)
- **RAM**: 8 GB (16 GB recommended for large files)
- **Storage**: 5 GB free space (2-4 GB for AI models)
- **Python**: 3.8+ with pip and venv support

### Dependencies
- **Qt 5.12+** for UI framework
- **GStreamer 1.14+** with multitrack support
- **PyTorch** (auto-installed, BSD-3 license)
- **Demucs** (auto-installed, MIT license)

## 🚀 Installation

### From Source (Recommended)
```bash
# Clone repository
git clone https://github.com/your-repo/AIMP-Clementine.git
cd AIMP-Clementine

# Set up Python environment
python3 -m venv stem_separation_env
source stem_separation_env/bin/activate
pip install torch demucs

# Build application
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### First-time Setup
1. Launch Clementine Extended
2. Open **View → AI Stem Mixer** 
3. Load any audio file
4. Click **🤖 Separate Current Track**
5. Wait for model download (one-time, ~2-4 GB)
6. Use preset buttons or manual controls for mixing

## 🎯 Usage Guide

### Basic Operation
1. **Load Track**: Open any supported audio file (MP3, FLAC, WAV, etc.)
2. **Separate Stems**: Click "🤖 Separate Current Track" button
3. **Wait for Processing**: Progress bar shows separation status (2-5 minutes typical)
4. **Mix Audio**: Use volume sliders and mute buttons for each stem
5. **Apply Presets**: Quick buttons for karaoke, instrumental, or single-stem modes

### Pro Tips
- **Better CPU = Faster Processing**: Separation time depends on track length and CPU power
- **Use Presets**: Quick access to common mixing scenarios
- **Volume Automation**: Combine multiple stems for creative mixing
- **Error Recovery**: Check Python output in case of processing issues

## 🔧 Technical Architecture

### Core Components
- **StemSeparator**: C++ QProcess orchestration with 15-minute timeout
- **GstStemEngine**: GStreamer multitrack mixing with zero-latency playback  
- **StemMixerWidget**: Qt-based professional mixing interface
- **Python Integration**: Demucs CLI with structured progress reporting

### Signal Flow
```
Audio File → Demucs AI → Individual Stems → GStreamer Mixer → Audio Output
                ↓              ↓                ↓
            Progress UI    Volume Controls    Real-time Mix
```

## 🐛 Known Issues

### Current Limitations  
- **Live Mode Disabled**: Real-time separation postponed for stability
- **Processing Time**: Large files (>6 minutes) may take 5-10 minutes to separate
- **Memory Usage**: Peak usage during separation can reach 4-6 GB
- **Model Download**: Initial setup requires stable internet for model downloads

### Planned Improvements (v0.0.5)
- Faster separation algorithms 
- Background processing queue
- Real-time live mode implementation
- Additional AI model options
- Cloud-based separation services

## 📜 License Compliance

Clementine Extended is fully compliant with **GNU GPL v3** licensing:

### GPL-Compatible Dependencies
- ✅ **PyTorch**: BSD 3-Clause License  
- ✅ **Demucs**: MIT License
- ✅ **ONNX Runtime**: MIT License  
- ✅ **TensorFlow Lite**: Apache 2.0 License

### Source Code Availability
- Full source code available at: [GitHub Repository]
- All modifications clearly documented
- License headers in all new files
- Comprehensive dependency analysis in `LICENSE_COMPLIANCE.md`

## 🤝 Contributing

We welcome contributions! Please:
1. Review our [GPL v3 license](COPYING) requirements
2. Ensure new code includes proper license headers
3. Test AI functionality with sample audio files
4. Submit pull requests with clear descriptions

## 📞 Support

- **Issues**: GitHub issue tracker
- **Discussions**: GitHub discussions page  
- **Documentation**: See `README.md` and wiki
- **License Questions**: Review `LICENSE_COMPLIANCE.md`

---

**⚠️ Experimental Release**: This version focuses on core offline stem separation. Some advanced features are still in development.

**🎵 Ready to Experience AI-Powered Music Mixing?** Download v0.0.4-experimental and transform how you listen to music!
