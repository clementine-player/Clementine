/* This file is part of Clementine.
   Copyright 2010, Paweł Bara <keirangtp@gmail.com>
   Copyright 2010-2011, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Alexander Bikadorov <abiku@cs.tu-berlin.de>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "config.h"
#include "globalshortcuts.h"
#include "gnomeglobalshortcutbackend.h"
#include "macglobalshortcutbackend.h"
#include "qxtglobalshortcutbackend.h"

#include "mac_startup.h"

#include <QAction>
#include <QShortcut>
#include <QSignalMapper>
#include <QtDebug>

#ifdef HAVE_DBUS
#include <QtDBus>
#endif

const char* GlobalShortcuts::kSettingsGroup = "Shortcuts";

GlobalShortcuts::GlobalShortcuts(QWidget* parent)
    : QWidget(parent),
      gnome_backend_(nullptr),
      system_backend_(nullptr),
      use_gnome_(false),
      rating_signals_mapper_(new QSignalMapper(this)) {
  settings_.beginGroup(kSettingsGroup);

  // Create actions
  AddShortcut("play", tr("Play"), SIGNAL(Play()));
  AddShortcut("pause", tr("Pause"), SIGNAL(Pause()));
  AddShortcut("play_pause", tr("Play/Pause"), SIGNAL(PlayPause()),
              QKeySequence(Qt::Key_MediaPlay));
  AddShortcut("stop", tr("Stop"), SIGNAL(Stop()),
              QKeySequence(Qt::Key_MediaStop));
  AddShortcut("stop_after", tr("Stop playing after current track"),
              SIGNAL(StopAfter()));
  AddShortcut("next_track", tr("Next track"), SIGNAL(Next()),
              QKeySequence(Qt::Key_MediaNext));
  AddShortcut("prev_track", tr("Previous track"), SIGNAL(Previous()),
              QKeySequence(Qt::Key_MediaPrevious));
  AddShortcut("inc_volume", tr("Increase volume"), SIGNAL(IncVolume()));
  AddShortcut("dec_volume", tr("Decrease volume"), SIGNAL(DecVolume()));
  AddShortcut("mute", tr("Mute"), SIGNAL(Mute()));
  AddShortcut("seek_forward", tr("Seek forward"), SIGNAL(SeekForward()));
  AddShortcut("seek_backward", tr("Seek backward"), SIGNAL(SeekBackward()));
  AddShortcut("show_hide", tr("Show/Hide"), SIGNAL(ShowHide()));
  AddShortcut("show_osd", tr("Show OSD"), SIGNAL(ShowOSD()));
  AddShortcut(
      "toggle_pretty_osd", tr("Toggle Pretty OSD"),
      SIGNAL(TogglePrettyOSD()));  // Toggling possible only for pretty OSD
  AddShortcut("shuffle_mode", tr("Change shuffle mode"),
              SIGNAL(CycleShuffleMode()));
  AddShortcut("repeat_mode", tr("Change repeat mode"),
              SIGNAL(CycleRepeatMode()));
  AddShortcut("toggle_last_fm_scrobbling",
              tr("Enable/disable Last.fm scrobbling"),
              SIGNAL(ToggleScrobbling()));
  AddShortcut("love_last_fm_scrobbling", tr("Love (Last.fm scrobbling)"),
              SIGNAL(Love()));
  AddShortcut("ban_last_fm_scrobbling", tr("Ban (Last.fm scrobbling)"),
              SIGNAL(Ban()));
  AddShortcut("remove_current_song_from_playlist",
              tr("Remove current song from playlist"),
              SIGNAL(RemoveCurrentSong()));

  AddRatingShortcut("rate_zero_star", tr("Rate the current song 0 stars"),
                    rating_signals_mapper_, 0);
  AddRatingShortcut("rate_one_star", tr("Rate the current song 1 star"),
                    rating_signals_mapper_, 1);
  AddRatingShortcut("rate_two_star", tr("Rate the current song 2 stars"),
                    rating_signals_mapper_, 2);
  AddRatingShortcut("rate_three_star", tr("Rate the current song 3 stars"),
                    rating_signals_mapper_, 3);
  AddRatingShortcut("rate_four_star", tr("Rate the current song 4 stars"),
                    rating_signals_mapper_, 4);
  AddRatingShortcut("rate_five_star", tr("Rate the current song 5 stars"),
                    rating_signals_mapper_, 5);

  connect(rating_signals_mapper_, SIGNAL(mapped(int)),
          SIGNAL(RateCurrentSong(int)));

  // Create backends - these do the actual shortcut registration
  gnome_backend_ = new GnomeGlobalShortcutBackend(this);

#ifndef Q_OS_DARWIN
  system_backend_ = new QxtGlobalShortcutBackend(this);
#else
  system_backend_ = new MacGlobalShortcutBackend(this);
#endif

  ReloadSettings();
}

void GlobalShortcuts::AddShortcut(const QString& id, const QString& name,
                                  const char* signal,
                                  const QKeySequence& default_key) {
  Shortcut shortcut = AddShortcut(id, name, default_key);
  connect(shortcut.action, SIGNAL(triggered()), this, signal);
}

void GlobalShortcuts::AddRatingShortcut(const QString& id, const QString& name,
                                        QSignalMapper* mapper, int rating,
                                        const QKeySequence& default_key) {
  Shortcut shortcut = AddShortcut(id, name, default_key);
  connect(shortcut.action, SIGNAL(triggered()), mapper, SLOT(map()));
  mapper->setMapping(shortcut.action, rating);
}

GlobalShortcuts::Shortcut GlobalShortcuts::AddShortcut(
    const QString& id, const QString& name, const QKeySequence& default_key) {
  Shortcut shortcut;
  shortcut.action = new QAction(name, this);
  QKeySequence key_sequence = QKeySequence::fromString(
      settings_.value(id, default_key.toString()).toString());
  shortcut.action->setShortcut(key_sequence);
  shortcut.id = id;
  shortcut.default_key = default_key;

  // Create application wide QShortcut to hide keyevents mapped to global
  // shortcuts from widgets.
  shortcut.shortcut = new QShortcut(key_sequence, this);
  shortcut.shortcut->setContext(Qt::ApplicationShortcut);

  shortcuts_[id] = shortcut;

  return shortcut;
}

bool GlobalShortcuts::IsGsdAvailable() const {
#ifdef HAVE_DBUS
  return QDBusConnection::sessionBus().interface()->isServiceRegistered(
             GnomeGlobalShortcutBackend::kGsdService) ||
         QDBusConnection::sessionBus().interface()->isServiceRegistered(
             GnomeGlobalShortcutBackend::kGsdService2);
#else  // HAVE_DBUS
  return false;
#endif
}

void GlobalShortcuts::ReloadSettings() {
  // The actual shortcuts have been set in our actions for us by the config
  // dialog already - we just need to reread the gnome settings.
  use_gnome_ = settings_.value("use_gnome", true).toBool();

  Unregister();
  Register();
}

void GlobalShortcuts::Unregister() {
  if (gnome_backend_->is_active()) gnome_backend_->Unregister();
  if (system_backend_->is_active()) system_backend_->Unregister();
}

void GlobalShortcuts::Register() {
  if (use_gnome_ && gnome_backend_->Register()) return;
  system_backend_->Register();
}

bool GlobalShortcuts::IsMacAccessibilityEnabled() const {
#ifdef Q_OS_MAC
  return static_cast<MacGlobalShortcutBackend*>(system_backend_)
      ->IsAccessibilityEnabled();
#else
  return true;
#endif
}

void GlobalShortcuts::ShowMacAccessibilityDialog() {
#ifdef Q_OS_MAC
  static_cast<MacGlobalShortcutBackend*>(system_backend_)
      ->ShowAccessibilityDialog();
#endif
}
