#ifndef IAUDIOBACKEND_H
#define IAUDIOBACKEND_H

// Forward declarations
class QString;
template<class T> class QList;
class QStringList;

namespace Engine {

// Audio output types
enum class OutputType {
  DirectSound,
  WASAPI_Shared,
  WASAPI_Exclusive,
  ASIO,
  ALSA,
  PulseAudio,
  FFmpeg
};

// Audio bit depth support
enum class BitDepth {
  Bit16 = 16,
  Bit24 = 24,
  Bit32 = 32
};

// Audio sample rates
enum class SampleRate {
  Rate44100 = 44100,
  Rate48000 = 48000,
  Rate96000 = 96000,
  Rate192000 = 192000
};

// Playback modes
enum class PlaybackMode {
  Standard,    // Use configured bit depth and sample rate
  BitPerfect   // Use source file's original format
};

class IAudioBackend {
public:
  virtual ~IAudioBackend() {}
  
  // Backend-Identifikation
  virtual QString Name() const = 0;
  virtual QString Description() const = 0;
  virtual bool IsAvailable() const = 0;
  
  // Erweiterte Backend-Features
  virtual OutputType GetOutputType() const = 0;
  virtual QStringList SupportedFormats() const = 0;
  virtual QList<BitDepth> SupportedBitDepths() const = 0;
  virtual QList<SampleRate> SupportedSampleRates() const = 0;
  
  // Audio Processing Capabilities
  virtual bool Supports32BitProcessing() const = 0;
  virtual bool SupportsExclusiveMode() const = 0;
  virtual bool SupportsLowLatency() const = 0;
  
  // Quality settings
  virtual void SetBitDepth(BitDepth depth) = 0;
  virtual BitDepth GetCurrentBitDepth() const = 0;
  virtual void SetSampleRate(SampleRate rate) = 0;
  virtual SampleRate GetCurrentSampleRate() const = 0;
  
  // Bitperfect playback
  virtual void SetPlaybackMode(PlaybackMode mode) = 0;
  virtual PlaybackMode GetPlaybackMode() const = 0;
  virtual bool SupportsBitPerfect() const = 0;
};

} // namespace Engine

#endif // IAUDIOBACKEND_H
