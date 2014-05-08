#include "remoteclient.h"

RemoteClient::RemoteClient(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      downloader_(false) {
}
