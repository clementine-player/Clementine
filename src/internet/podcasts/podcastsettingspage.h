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

#ifndef PODCASTS_PODCASTSETTINGSPAGE_H_
#define PODCASTS_PODCASTSETTINGSPAGE_H_

#include "ui/settingspage.h"

class Ui_PodcastSettingsPage;

class QNetworkReply;

class PodcastSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit PodcastSettingsPage(SettingsDialog* dialog);
  ~PodcastSettingsPage();

  static const char* kSettingsGroup;

  void Load();
  void Save();

 private slots:
  void LoginClicked();
  void LoginFinished(QNetworkReply* reply);
  void LogoutClicked();

  void DownloadDirBrowse();

 private:
  Ui_PodcastSettingsPage* ui_;
};

#endif  // PODCASTS_PODCASTSETTINGSPAGE_H_
