#include "globalshortcuts.h"
#include "qxtglobalshortcut.h"

#include <QtDBus>

const char* GlobalShortcuts::kGsdService = "org.gnome.SettingsDaemon";
const char* GlobalShortcuts::kGsdPath = "/org/gnome/SettingsDaemon/MediaKeys";
const char* GlobalShortcuts::kGsdInterface = "org.gnome.SettingsDaemon.MediaKeys";

GlobalShortcuts::GlobalShortcuts(QObject *parent)
  : QObject(parent)
{
  Init();
}

void GlobalShortcuts::Init() {
  if (RegisterGnome()) return;

#ifdef Q_WS_X11
  if (RegisterX11()) return;
#endif
}

bool GlobalShortcuts::RegisterGnome() {
  // Check if the GSD service is available
  if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(kGsdService))
    return false;

  QDBusInterface* interface = new QDBusInterface(
      kGsdService, kGsdPath, kGsdInterface, QDBusConnection::sessionBus(), this);

  connect(interface, SIGNAL(MediaPlayerKeyPressed(QString,QString)),
          this, SLOT(GnomeMediaKeyPressed(QString,QString)));

  return true;
}

bool GlobalShortcuts::RegisterX11() {
  QxtGlobalShortcut* play_pause = new QxtGlobalShortcut(QKeySequence("Media Play"), this);
  QxtGlobalShortcut* stop = new QxtGlobalShortcut(QKeySequence("Media Stop"), this);
  QxtGlobalShortcut* next = new QxtGlobalShortcut(QKeySequence("Media Next"), this);
  QxtGlobalShortcut* prev = new QxtGlobalShortcut(QKeySequence("Media Previous"), this);

  connect(play_pause, SIGNAL(activated()), SIGNAL(PlayPause()));
  connect(stop, SIGNAL(activated()), SIGNAL(Stop()));
  connect(next, SIGNAL(activated()), SIGNAL(Next()));
  connect(prev, SIGNAL(activated()), SIGNAL(Previous()));
}

void GlobalShortcuts::GnomeMediaKeyPressed(const QString&, const QString& key) {
  if (key == "Play")     emit PlayPause();
  if (key == "Stop")     emit Stop();
  if (key == "Next")     emit Next();
  if (key == "Previous") emit Previous();
}
