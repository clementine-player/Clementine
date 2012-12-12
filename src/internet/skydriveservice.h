#ifndef SKYDRIVESERVICE_H
#define SKYDRIVESERVICE_H

#include "cloudfileservice.h"

class OAuthenticator;

class SkydriveService : public CloudFileService {
  Q_OBJECT

 public:
  SkydriveService(
      Application* app,
      InternetModel* parent);

 protected:
  // CloudFileService
  virtual bool has_credentials() const;
  virtual void Connect();

 private slots:
  void ConnectFinished(OAuthenticator* oauth);

};

#endif  // SKYDRIVESERVICE_H
