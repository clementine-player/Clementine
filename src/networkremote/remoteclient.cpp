#include "remoteclient.h"

#include "networkremote/songsender.h"

RemoteClient::RemoteClient(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      downloader_(false),
      song_sender_(new SongSender(app, this)) {
}
