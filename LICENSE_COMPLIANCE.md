# 📋 Open Source License Compliance Report

## Current Project License
- **Clementine**: GNU GPL v3 (GPLv3)
- **File**: COPYING
- **Requirement**: All dependencies must be GPL-compatible

## ✅ Approved Libraries for Stem Separation

### Option 1: PyTorch/LibTorch
- **License**: BSD-3-Clause 
- **✅ GPL Compatible**: Yes (BSD is GPL-compatible)
- **Open Source**: ✅ Full source available
- **Repository**: https://github.com/pytorch/pytorch
- **Download**: https://pytorch.org/cppdocs/installing.html

### Option 2: ONNX Runtime (Recommended)
- **License**: MIT License
- **✅ GPL Compatible**: Yes (MIT is GPL-compatible) 
- **Open Source**: ✅ Full source available
- **Repository**: https://github.com/microsoft/onnxruntime
- **Advantage**: Smaller footprint, easier integration

### Option 3: TensorFlow Lite C++
- **License**: Apache 2.0
- **✅ GPL Compatible**: Yes (Apache 2.0 is GPL-compatible)
- **Open Source**: ✅ Full source available
- **Repository**: https://github.com/tensorflow/tensorflow

## ✅ Model Compatibility

### Demucs Models
- **License**: MIT License
- **✅ GPL Compatible**: Yes
- **Open Source**: ✅ Full source available
- **Repository**: https://github.com/facebookresearch/demucs
- **Models**: Can be converted to ONNX format

## 🚫 Libraries to AVOID
- Any proprietary/closed-source libraries
- Libraries with incompatible licenses (some commercial licenses)
- Virtual DJ SDK (proprietary)

## 📝 Recommendation

**Use ONNX Runtime + Demucs ONNX models**:
1. ✅ Fully GPL-compatible (MIT License)
2. ✅ Smaller footprint than LibTorch
3. ✅ Easier C++ integration
4. ✅ Official Demucs ONNX support available
5. ✅ Better for distribution

## Implementation Plan
1. Download pre-trained Demucs ONNX models
2. Integrate ONNX Runtime C++ API
3. Create native C++ stem separator
4. Replace Python dependency completely

This approach ensures 100% license compliance while achieving our goal!
