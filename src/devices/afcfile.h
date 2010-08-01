#ifndef AFCFILE_H
#define AFCFILE_H

#include <stdint.h>

#include <QIODevice>
#include <QUrl>

struct afc_client_private;
typedef afc_client_private* afc_client_t;

class AFCFile : public QIODevice {
  Q_OBJECT
 public:
  AFCFile(afc_client_t client, const QUrl& url, QObject* parent = 0);
  ~AFCFile();

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

  QUrl url_;
};

#endif
