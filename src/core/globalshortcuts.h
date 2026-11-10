/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2010, Paweł Bara <keirangtp@gmail.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
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

#ifndef CORE_GLOBALSHORTCUTS_H_
#define CORE_GLOBALSHORTCUTS_H_

#include <QKeySequence>
#include <QMap>
#include <QSettings>
#include <QWidget>

class QAction;
class QShortcut;

class GlobalShortcutBackend;
class PortalGlobalShortcutBackend;

class GlobalShortcuts : public QWidget {
  Q_OBJECT

 public:
  explicit GlobalShortcuts(QWidget* parent = nullptr);

  static const char* kSettingsGroup;

  struct Shortcut {
    QString id;
    QKeySequence default_key;
    QAction* action;
    QShortcut* shortcut;
  };

  QMap<QString, Shortcut> shortcuts() const { return shortcuts_; }
  bool IsGsdAvailable() const;
  bool IsMacAccessibilityEnabled() const;
  // Whether the desktop portal handles the shortcuts, so they're changed in
  // the desktop's settings.
  bool IsPortalActive() const;
  // Whether the user has set up the portal's shortcuts. Until then they're
  // not bound, so the desktop doesn't ask about them at every start.
  bool IsPortalSetUp() const;
  bool CanConfigurePortal() const;

 public slots:
  void ReloadSettings();
  void ShowMacAccessibilityDialog();
  void ConfigurePortal();
  // Binds the portal's shortcuts, so the desktop may ask the user about them.
  void SetUpPortal();

  void Unregister();
  void Register();

 signals:
  void Play();
  void Pause();
  void PlayPause();
  void Stop();
  void StopAfter();
  void Next();
  void NextAlbum();
  void Previous();
  void IncVolume();
  void DecVolume();
  void Mute();
  void SeekForward();
  void SeekBackward();
  void ShowHide();
  void ShowOSD();
  void TogglePrettyOSD();
  void RateCurrentSong(int);
  void CycleShuffleMode();
  void CycleRepeatMode();
  void ToggleScrobbling();
  void Love();
  void Ban();
  void RemoveCurrentSong();

  void PortalSetUpFinished(bool success);

 private:
  void AddShortcut(const QString& id, const QString& name, const char* signal,
                   const QKeySequence& default_key = QKeySequence(0));
  void AddRatingShortcut(const QString& id, const QString& name, int rating,
                         const QKeySequence& default_key = QKeySequence(0));
  Shortcut AddShortcut(const QString& id, const QString& name,
                       const QKeySequence& default_key);

 private:
  GlobalShortcutBackend* gnome_backend_;
  GlobalShortcutBackend* kglobalaccel_backend_;
  PortalGlobalShortcutBackend* portal_backend_;
  GlobalShortcutBackend* system_backend_;

  QMap<QString, Shortcut> shortcuts_;
  QSettings settings_;

  bool use_gnome_;
  bool have_kglobalaccel_;
};

#endif  // CORE_GLOBALSHORTCUTS_H_
