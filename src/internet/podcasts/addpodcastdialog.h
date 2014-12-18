/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
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

#ifndef INTERNET_PODCASTS_ADDPODCASTDIALOG_H_
#define INTERNET_PODCASTS_ADDPODCASTDIALOG_H_

#include "podcast.h"

#include <QDialog>

class AddPodcastByUrl;
class AddPodcastPage;
class Application;
class OpmlContainer;
class WidgetFadeHelper;
class Ui_AddPodcastDialog;

class QModelIndex;

class AddPodcastDialog : public QDialog {
  Q_OBJECT

 public:
  explicit AddPodcastDialog(Application* app, QWidget* parent = nullptr);
  ~AddPodcastDialog();

  static const char* kBbcOpmlUrl;

  // Convenience methods that open the dialog at the Add By Url page and fill
  // it with either a URL (which is then fetched), or a pre-fetched OPML
  // container.
  void ShowWithUrl(const QUrl& url);
  void ShowWithOpml(const OpmlContainer& opml);

 private slots:
  void OpenSettingsPage();
  void AddPodcast();
  void PodcastDoubleClicked(const QModelIndex& index);
  void RemovePodcast();
  void ChangePage(int index);
  void ChangePodcast(const QModelIndex& current);

  void PageBusyChanged(bool busy);
  void CurrentPageBusyChanged(bool busy);

  void SelectFirstPodcast();

  void OpenOPMLFile();

 private:
  void AddPage(AddPodcastPage* page);

 private:
  Application* app_;

  Ui_AddPodcastDialog* ui_;
  QPushButton* add_button_;
  QPushButton* remove_button_;

  QList<AddPodcastPage*> pages_;
  QList<bool> page_is_busy_;
  AddPodcastByUrl* by_url_page_;

  WidgetFadeHelper* fader_;

  Podcast current_podcast_;

  QString last_opml_path_;
};

#endif  // INTERNET_PODCASTS_ADDPODCASTDIALOG_H_
