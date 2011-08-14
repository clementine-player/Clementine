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

#include "config.h"
#include "widgets/osd.h"

class QScrollArea;

class BackgroundStreams;
class GlobalShortcuts;
class LibraryDirectoryModel;
class SettingsPage;
class SongInfoView;
class Ui_SettingsDialog;

class GstEngine;

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  SettingsDialog(BackgroundStreams* streams, QWidget* parent = 0);
  ~SettingsDialog();

  enum Page {
    Page_Playback,
    Page_Behaviour,
    Page_SongInformation,
    Page_GlobalShortcuts,
    Page_Notifications,
    Page_Library,
    Page_Lastfm,
    Page_Spotify,
    Page_Magnatune,
    Page_DigitallyImported,
    Page_BackgroundStreams,
    Page_Proxy,
    Page_Transcoding,
    Page_Remote,
    Page_Wiimotedev,
  };

  void SetLibraryDirectoryModel(LibraryDirectoryModel* model) { model_ = model; }
  void SetGlobalShortcutManager(GlobalShortcuts* manager) { manager_ = manager; }
  void SetGstEngine(const GstEngine* engine) { gst_engine_ = engine; }
  void SetSongInfoView(SongInfoView* view) { song_info_view_ = view; }

  bool is_loading_settings() const { return loading_settings_; }

  LibraryDirectoryModel* library_directory_model() const { return model_; }
  GlobalShortcuts* global_shortcuts_manager() const { return manager_; }
  const GstEngine* gst_engine() const { return gst_engine_; }
  SongInfoView* song_info_view() const { return song_info_view_; }
  BackgroundStreams* background_streams() const { return streams_; }

  void OpenAtPage(Page page);

  // QDialog
  void accept();

  // QWidget
  void showEvent(QShowEvent* e);

signals:
  void NotificationPreview(OSD::Behaviour, QString, QString);
  void SetWiimotedevInterfaceActived(bool);

private slots:
  void CurrentTextChanged(const QString& text);

private:
  struct PageData {
    int index_;
    QScrollArea* scroll_area_;
    SettingsPage* page_;
  };

  void AddPage(Page id, SettingsPage* page);

private:
  LibraryDirectoryModel* model_;
  GlobalShortcuts* manager_;
  const GstEngine* gst_engine_;
  SongInfoView* song_info_view_;
  BackgroundStreams* streams_;

  Ui_SettingsDialog* ui_;
  bool loading_settings_;

  QMap<Page, PageData> pages_;
};

#endif // SETTINGSDIALOG_H
