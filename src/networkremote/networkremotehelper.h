#ifndef NETWORKREMOTEHELPER_H
#define NETWORKREMOTEHELPER_H

#include <QObject>

class Application;

class NetworkRemoteHelper : public QObject {
  Q_OBJECT
 public:
  static NetworkRemoteHelper* Instance();

  NetworkRemoteHelper(Application* app);
  ~NetworkRemoteHelper();

  void ReloadSettings();

 private slots:
  void StartServer();

signals:
  void SetupServerSig();
  void StartServerSig();
  void ReloadSettingsSig();

 private:
  static NetworkRemoteHelper* sInstance;
  Application* app_;
};

#endif  // NETWORKREMOTEHELPER_H
