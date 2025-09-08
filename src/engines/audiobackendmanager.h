#ifndef AUDIOBACKENDMANAGER_H
#define AUDIOBACKENDMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>

// Forward declaration
namespace Engine {
  class Base;
  class IAudioBackend;
}

namespace Engine {

class AudioBackendManager : public QObject {
  Q_OBJECT
public:
  static AudioBackendManager* instance();

  void registerBackend(IAudioBackend* backend);
  Base* currentBackend() const;
  void setCurrentBackend(const QString& name);
  QStringList availableBackends() const;
  IAudioBackend* getBackend(const QString& name) const;

signals:
  void backendChanged(const QString& name);

private:
  explicit AudioBackendManager(QObject* parent = nullptr);
  QMap<QString, IAudioBackend*> backends_;
  Base* current_backend_;
};

} // namespace Engine

#endif // AUDIOBACKENDMANAGER_H
