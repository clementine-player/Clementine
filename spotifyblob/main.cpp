#include <QCoreApplication>
#include <QStringList>

#include "spotifyblob.h"
#include "spotifyclient.h"
#include "core/logging.h"

int main(int argc, char** argv) {
  QCoreApplication a(argc, argv);

  logging::Init();

  const QStringList arguments(a.arguments());

  if (arguments.length() != 2) {
    qFatal("Usage: %s port", argv[0]);
  }

  SpotifyClient client;
  SpotifyBlob blob(&client);

  client.Init(arguments[1].toInt());

  return a.exec();
}
