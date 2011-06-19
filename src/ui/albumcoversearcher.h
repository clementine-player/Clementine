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

#ifndef ALBUMCOVERSEARCHER_H
#define ALBUMCOVERSEARCHER_H

#include "core/backgroundthread.h"
#include "covers/albumcoverfetcher.h"
#include "widgets/kcategorydrawer.h"

#include <QDialog>
#include <QIcon>

#include <boost/shared_ptr.hpp>

class AlbumCoverLoader;
class Ui_AlbumCoverSearcher;

class QModelIndex;
class QStandardItem;
class QStandardItemModel;

class AlbumCoverCategoryDrawer : public KCategoryDrawerV3 {
public:
  AlbumCoverCategoryDrawer(KCategorizedView* view);

  int categoryHeight(const QModelIndex& index, const QStyleOption& option) const;
  void drawCategory(const QModelIndex& index, int sortRole,
                    const QStyleOption& option, QPainter* painter) const;

private:
  static const int kBarThickness;
  static const int kBarMarginTop;
  static const int kBarMarginBottom;

  int total_height_;
};


// This is a dialog that lets the user search for album covers
class AlbumCoverSearcher : public QDialog {
  Q_OBJECT

public:
  AlbumCoverSearcher(const QIcon& no_cover_icon, QWidget* parent);
  ~AlbumCoverSearcher();

  enum Role {
    Role_ImageURL = Qt::UserRole + 1,
    Role_ImageRequestId,
    Role_ImageFetchFinished,
  };

  void Init(AlbumCoverFetcher* fetcher);

  QImage Exec(const QString& query);

protected:
  void keyPressEvent(QKeyEvent *);

private slots:
  void Search();
  void SearchFinished(quint64 id, const CoverSearchResults& results);
  void ImageLoaded(quint64 id, const QImage& image);

  void CoverDoubleClicked(const QModelIndex& index);

private:
  Ui_AlbumCoverSearcher* ui_;

  QStandardItemModel* model_;

  QIcon no_cover_icon_;
  BackgroundThread<AlbumCoverLoader>* loader_;
  AlbumCoverFetcher* fetcher_;

  quint64 id_;
  QMap<quint64, QStandardItem*> cover_loading_tasks_;
};

#endif // ALBUMCOVERSEARCHER_H
