/* This file is part of Clementine.
   Copyright 2018, Jim Broadus <jbroadus@gmail.com>

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

#ifndef INTERNET_PODCASTS_PODCASTINFODIALOG_H_
#define INTERNET_PODCASTS_PODCASTINFODIALOG_H_

#include <QDialog>

class Application;
class Podcast;
class Ui_PodcastInfoDialog;

class PodcastInfoDialog : public QDialog {
  Q_OBJECT

 public:
  explicit PodcastInfoDialog(Application* app, QWidget* parent = nullptr);
  ~PodcastInfoDialog();

  void ShowPodcast(const Podcast& podcast);

 private:
  Application* app_;

  Ui_PodcastInfoDialog* ui_;
};

#endif  // INTERNET_PODCASTS_PODCASTINFODIALOG_H_
