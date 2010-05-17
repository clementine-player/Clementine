/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "globalshortcuts.h"
#include "gnomeglobalshortcutbackend.h"
#include "qxtglobalshortcutbackend.h"

#include "mac_startup.h"

#include <QtDebug>
#include <QAction>

#ifdef QT_DBUS_LIB
#  include <QtDBus>
#endif

const char* GlobalShortcuts::kSettingsGroup = "Shortcuts";

GlobalShortcuts::GlobalShortcuts(QObject *parent)
  : QObject(parent),
    gnome_backend_(NULL),
    system_backend_(NULL)
{
  settings_.beginGroup(kSettingsGroup);

  // Create actions
  AddShortcut("play", tr("Play"), SIGNAL(Play()));
  AddShortcut("pause", tr("Pause"), SIGNAL(Pause()));
  AddShortcut("play_pause", tr("Play/Pause"), SIGNAL(PlayPause()), QKeySequence(Qt::Key_MediaPlay));
  AddShortcut("stop", tr("Stop"), SIGNAL(Stop()), QKeySequence(Qt::Key_MediaStop));
  AddShortcut("stop_after", tr("Stop playing after current track"), SIGNAL(StopAfter()));
  AddShortcut("next_track", tr("Next track"), SIGNAL(Next()), QKeySequence(Qt::Key_MediaNext));
  AddShortcut("prev_track", tr("Previous track"), SIGNAL(Previous()), QKeySequence(Qt::Key_MediaPrevious));
  AddShortcut("inc_volume", tr("Increase volume"), SIGNAL(IncVolume()));
  AddShortcut("dec_volume", tr("Decrease volume"), SIGNAL(DecVolume()));
  AddShortcut("mute", tr("Mute"), SIGNAL(Mute()));
  AddShortcut("seek_forward", tr("Seek forward"), SIGNAL(SeekForward()));
  AddShortcut("seek_backward", tr("Seek backward"), SIGNAL(SeekBackward()));

  // Create backends - these do the actual shortcut registration
  if (IsGsdAvailable())
    gnome_backend_ = new GnomeGlobalShortcutBackend(this);

#ifndef Q_OS_DARWIN
  system_backend_ = new QxtGlobalShortcutBackend(this);
#endif

  ReloadSettings();
}

void GlobalShortcuts::AddShortcut(const QString &id, const QString &name,
                                  const char* signal,
                                  const QKeySequence &default_key) {
  Shortcut shortcut;
  shortcut.action = new QAction(name, this);
  shortcut.action->setShortcut(QKeySequence::fromString(
      settings_.value(id, default_key.toString()).toString()));
  shortcut.id = id;
  shortcut.default_key = default_key;

  connect(shortcut.action, SIGNAL(triggered()), this, signal);

  shortcuts_[id] = shortcut;
}

bool GlobalShortcuts::IsGsdAvailable() const {
#ifdef QT_DBUS_LIB
  return QDBusConnection::sessionBus().interface()->isServiceRegistered(
      GnomeGlobalShortcutBackend::kGsdService);
#else // QT_DBUS_LIB
  return false;
#endif
}

void GlobalShortcuts::MacMediaKeyPressed(const QString& key) {
  if (key == "Play")     emit PlayPause();
  // Stop doesn't exist on a mac keyboard.
  if (key == "Next")     emit Next();
  if (key == "Previous") emit Previous();
}

void GlobalShortcuts::ReloadSettings() {
  // The actual shortcuts have been set in our actions for us by the config
  // dialog already - we just need to reread the gnome settings.
  bool use_gnome = settings_.value("use_gnome", true).toBool();

  if (gnome_backend_ && gnome_backend_->is_active())
    gnome_backend_->Unregister();
  if (system_backend_ && system_backend_->is_active())
    system_backend_->Unregister();

  if (gnome_backend_ && use_gnome)
    gnome_backend_->Register();
  else if (system_backend_)
    system_backend_->Register();
}
