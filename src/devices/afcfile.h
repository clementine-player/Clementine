#ifndef AFCFILE_H
#define AFCFILE_H

#include <stdint.h>

#include <QIODevice>

#include <libimobiledevice/afc.h>

class iMobileDeviceConnection;

class AfcFile : public QIODevice {
  Q_OBJECT

public:
  AfcFile(afc_client_t client, const QString& path, QObject* parent = 0);
  AfcFile(iMobileDeviceConnection* connection, const QString& path, QObject* parent = 0);
  ~AfcFile();

  // QIODevice
  void close();
  bool open(OpenMode mode);
  bool seek(qint64 pos);

private:
  // QIODevice
  qint64 readData(char* data, qint64 max_size);
  qint64 writeData(const char* data, qint64 max_size);

  afc_client_t client_;
  uint64_t handle_;

  QString path_;
};

#endif
