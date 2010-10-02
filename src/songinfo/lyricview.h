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

#ifndef LYRICVIEW_H
#define LYRICVIEW_H

#include "songinfobase.h"
#include "core/song.h"

class LyricFetcher;
class NetworkAccessManager;
class Ui_LyricView;

class LyricView : public SongInfoBase {
  Q_OBJECT

public:
  LyricView(QWidget* parent = 0);
  ~LyricView();

  void set_network(NetworkAccessManager* network);
  LyricFetcher* fetcher() const { return fetcher_; }

protected:
  void Update(const Song& metadata);

private slots:
  void SearchProgress(int id, const QString& provider);
  void SearchFinished(int id, bool success, const QString& title, const QString& content);

private:
  Ui_LyricView* ui_;

  LyricFetcher* fetcher_;
  int current_request_id_;
};

#endif // LYRICVIEW_H
