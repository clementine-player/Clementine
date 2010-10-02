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

#ifndef ARTISTINFOVIEW_H
#define ARTISTINFOVIEW_H

#include "songinfobase.h"

class ArtistInfoFetcher;

class QScrollArea;
class QVBoxLayout;

class ArtistInfoView : public SongInfoBase {
  Q_OBJECT

public:
  ArtistInfoView(QWidget* parent = 0);
  ~ArtistInfoView();

protected:
  void Update(const Song& metadata);

private:
  void AddChild(QWidget* widget);
  void Clear();

private slots:
  void ImageReady(int id, const QUrl& url);
  void InfoReady(int id, const QString& title, QWidget* widget);

private:
  ArtistInfoFetcher* fetcher_;
  int current_request_id_;

  QScrollArea* scroll_area_;
  QVBoxLayout* container_;
  QList<QWidget*> children_;
};

#endif // ARTISTINFOVIEW_H
