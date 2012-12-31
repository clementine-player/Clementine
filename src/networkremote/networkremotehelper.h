#ifndef NETWORKREMOTEHELPER_H
#define NETWORKREMOTEHELPER_H

#include <QThread>

#include "networkremote.h"

class NetworkRemoteHelper : public QObject {
    Q_OBJECT
public:
  static NetworkRemoteHelper* Instance();

  NetworkRemoteHelper(Application* app);
  ~NetworkRemoteHelper();

  void StartServer();
  void ReloadSettings();

signals:
  void SetupServerSig();
  void StartServerSig();
  void ReloadSettingsSig();

private:
  static NetworkRemoteHelper* sInstance;
  Application* app_;
};

#endif // NETWORKREMOTEHELPER_H
