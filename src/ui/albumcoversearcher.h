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

#ifndef ALBUMCOVERSEARCHER_H
#define ALBUMCOVERSEARCHER_H

#include "core/albumcoverfetcher.h"

#include <QDialog>

#include <boost/shared_ptr.hpp>

class AlbumCoverLoader;
class AlbumCoverManager;
class Ui_AlbumCoverSearcher;

class QListWidgetItem;
class QModelIndex;

class AlbumCoverSearcher : public QDialog {
  Q_OBJECT

public:
  AlbumCoverSearcher(AlbumCoverManager* parent);
  ~AlbumCoverSearcher();

  enum Role {
    Role_ImageURL = Qt::UserRole + 1,
    Role_ImageRequestId,
  };

  void Init(boost::shared_ptr<AlbumCoverLoader> loader,
            AlbumCoverFetcher* fetcher);

  QImage Exec(const QString& query);

protected:
  void keyPressEvent(QKeyEvent *);

private slots:
  void Search();
  void SearchFinished(quint64 id, const AlbumCoverFetcher::SearchResults& results);
  void ImageLoaded(quint64 id, const QImage& image);

  void CoverDoubleClicked(const QModelIndex& index);

private:
  Ui_AlbumCoverSearcher* ui_;

  AlbumCoverManager* manager_;
  boost::shared_ptr<AlbumCoverLoader> loader_;
  AlbumCoverFetcher* fetcher_;

  quint64 id_;
  QMap<quint64, QListWidgetItem*> cover_loading_tasks_;
};

#endif // ALBUMCOVERSEARCHER_H
