#ifndef DROPBOXSERVICE_H
#define DROPBOXSERVICE_H

#include "internet/cloudfileservice.h"

#include "core/tagreaderclient.h"

class DropboxAuthenticator;

class DropboxService : public CloudFileService {
  Q_OBJECT
 public:
  DropboxService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;

 signals:
  void Connected();

 public slots:
  void Connect();

 private slots:
  void AuthenticationFinished(DropboxAuthenticator* authenticator);

 private:
  QString access_token_;
  QString access_token_secret_;
};

#endif  // DROPBOXSERVICE_H
