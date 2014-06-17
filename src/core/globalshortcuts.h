/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

#include <QKeySequence>
#include <QMap>
#include <QSettings>
#include <QWidget>

class QAction;
class QShortcut;

class GlobalShortcutBackend;
class QSignalMapper;

class GlobalShortcuts : public QWidget {
  Q_OBJECT

 public:
  GlobalShortcuts(QWidget* parent = nullptr);

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

 public slots:
  void ReloadSettings();
  void ShowMacAccessibilityDialog();

  void Unregister();
  void Register();

signals:
  void Play();
  void Pause();
  void PlayPause();
  void Stop();
  void StopAfter();
  void Next();
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
  void DeletePlaylistSelected();

 private:
  void AddShortcut(const QString& id, const QString& name, const char* signal,
                   const QKeySequence& default_key = QKeySequence(0));
  void AddRatingShortcut(const QString& id, const QString& name,
                         QSignalMapper* mapper, int rating,
                         const QKeySequence& default_key = QKeySequence(0));
  Shortcut AddShortcut(const QString& id, const QString& name,
                       const QKeySequence& default_key);

 private:
  GlobalShortcutBackend* gnome_backend_;
  GlobalShortcutBackend* system_backend_;

  QMap<QString, Shortcut> shortcuts_;
  QSettings settings_;

  bool use_gnome_;
  QSignalMapper* rating_signals_mapper_;
};

#endif
