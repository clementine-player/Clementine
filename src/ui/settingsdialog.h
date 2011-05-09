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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QUrl>

#include "config.h"

class QBoxLayout;
class QCheckBox;
class QSlider;

class BackgroundStreams;
class GlobalShortcuts;
class LibraryDirectoryModel;
class OSDPretty;
class SongInfoView;
class Ui_SettingsDialog;

#ifdef HAVE_LIBLASTFM
  class LastFMConfig;
#endif
#ifdef HAVE_WIIMOTEDEV
  class WiimotedevShortcutsConfig;
#endif
#ifdef HAVE_REMOTE
  class RemoteConfig;
#endif
#ifdef HAVE_SPOTIFY
  class SpotifyConfig;
#endif

class GstEngine;

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  SettingsDialog(BackgroundStreams* streams, QWidget* parent = 0);
  ~SettingsDialog();

  enum Page {
    Page_Playback = 0,
    Page_Behaviour,
    Page_SongInformation,
    Page_GlobalShortcuts,
    Page_Notifications,
    Page_Library,
#ifdef HAVE_LIBLASTFM
    Page_Lastfm,
#endif
#ifdef HAVE_SPOTIFY
    Page_Spotify,
#endif
    Page_Magnatune,
    Page_BackgroundStreams,
    Page_Proxy,
    Page_Transcoding,
#ifdef HAVE_REMOTE
    Page_Remote,
#endif
#ifdef HAVE_WIIMOTEDEV
    Page_Wiimotedev,
#endif
  };

  void SetLibraryDirectoryModel(LibraryDirectoryModel* model);
  void SetGlobalShortcutManager(GlobalShortcuts* manager);
  void SetGstEngine(const GstEngine* engine) { gst_engine_ = engine; }
  void SetSongInfoView(SongInfoView* view);

  void OpenAtPage(Page page);

  // QDialog
  void accept();

  // QWidget
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent *);

 private:
  void AddStream(const QString& name);
  void AddStreams();
  void LoadStreams();

 private slots:
  void CurrentTextChanged(const QString& text);
  void NotificationTypeChanged();

  void PrettyOpacityChanged(int value);
  void PrettyColorPresetChanged(int index);
  void ChooseBgColor();
  void ChooseFgColor();

  void UpdatePopupVisible();
  void ShowTrayIconToggled(bool on);
  void GstPluginChanged(int index);
  void FadingOptionsChanged();
  void RgPreampChanged(int value);

  void SongInfoFontSizeChanged(double value);

  // Background streams.
  void EnableStream(bool enabled);
  void StreamVolumeChanged(int value);

 private:
#ifdef HAVE_LIBLASTFM
  LastFMConfig* lastfm_config_;
#endif
#ifdef HAVE_WIIMOTEDEV
  WiimotedevShortcutsConfig* wiimotedev_config_;
#endif
#ifdef HAVE_REMOTE
  RemoteConfig* remote_config_;
#endif
#ifdef HAVE_SPOTIFY
  SpotifyConfig* spotify_config_;
#endif
  const GstEngine* gst_engine_;

  Ui_SettingsDialog* ui_;
  bool loading_settings_;

  OSDPretty* pretty_popup_;

  QMap<QString, QString> language_map_;

  BackgroundStreams* streams_;

#ifdef HAVE_WIIMOTEDEV
 signals:
  void SetWiimotedevInterfaceActived(bool);
#endif
};

#endif // SETTINGSDIALOG_H
