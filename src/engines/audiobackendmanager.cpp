#include "audiobackendmanager.h"
#include "iaudiobackend.h"
#include "enginebase.h"
#include "core/logging.h"

namespace Engine {

AudioBackendManager::AudioBackendManager(QObject* parent)
    : QObject(parent), current_backend_(nullptr) {}

AudioBackendManager* AudioBackendManager::instance() {
  static AudioBackendManager* instance = new AudioBackendManager();
  return instance;
}

void AudioBackendManager::registerBackend(IAudioBackend* backend) {
  backends_[backend->Name()] = backend;
}

Base* AudioBackendManager::currentBackend() const {
  return current_backend_;
}

void AudioBackendManager::setCurrentBackend(const QString& name) {
  if (backends_.contains(name)) {
    current_backend_ = dynamic_cast<Base*>(backends_[name]);
    
    // Apply backend configuration to the engine
    IAudioBackend* backend = backends_[name];
    if (backend) {
      // Apply current backend settings
      backend->SetPlaybackMode(backend->GetPlaybackMode());
      backend->SetBitDepth(backend->GetCurrentBitDepth());
      backend->SetSampleRate(backend->GetCurrentSampleRate());
      
      qLog(Info) << "AudioBackendManager: Switched to backend" << name 
                 << "with PlaybackMode:" << (backend->GetPlaybackMode() == Engine::PlaybackMode::BitPerfect ? "BitPerfect" : "Standard")
                 << "BitDepth:" << (int)backend->GetCurrentBitDepth()
                 << "SampleRate:" << (int)backend->GetCurrentSampleRate();
    }
    
    emit backendChanged(name);
  }
}

QStringList AudioBackendManager::availableBackends() const {
  return backends_.keys();
}

Engine::IAudioBackend* AudioBackendManager::getBackend(const QString& name) const {
  return backends_.value(name, nullptr);
}

} // namespace Engine
