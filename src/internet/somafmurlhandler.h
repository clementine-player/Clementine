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

#ifndef SOMAFMURLHANDLER_H
#define SOMAFMURLHANDLER_H

#include "core/urlhandler.h"

class Application;
class SomaFMService;


class SomaFMUrlHandler : public UrlHandler {
  Q_OBJECT

public:
  SomaFMUrlHandler(Application* app, SomaFMService* service, QObject* parent);

  QString scheme() const { return "somafm"; }
  QIcon icon() const { return QIcon(":providers/somafm.png"); }
  LoadResult StartLoading(const QUrl& url);

private slots:
  void LoadPlaylistFinished();

private:
  Application* app_;
  SomaFMService* service_;

  int task_id_;
};

#endif // SOMAFMURLHANDLER_H
