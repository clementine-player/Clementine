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

#include <QDialog>
#include <QIcon>
#include <QStyledItemDelegate>

#include "covers/albumcoverfetcher.h"
#include "covers/albumcoverloaderoptions.h"

class AlbumCoverLoader;
class Application;
class Ui_AlbumCoverSearcher;

class QModelIndex;
class QStandardItem;
class QStandardItemModel;

class SizeOverlayDelegate : public QStyledItemDelegate {
 public:
  static const int kMargin;
  static const int kPaddingX;
  static const int kPaddingY;
  static const qreal kBorder;
  static const qreal kFontPointSize;
  static const int kBorderAlpha;
  static const int kBackgroundAlpha;

  SizeOverlayDelegate(QObject* parent = NULL);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const;
};

// This is a dialog that lets the user search for album covers
class AlbumCoverSearcher : public QDialog {
  Q_OBJECT

 public:
  AlbumCoverSearcher(const QIcon& no_cover_icon, Application* app,
                     QWidget* parent);
  ~AlbumCoverSearcher();

  enum Role {
    Role_ImageURL = Qt::UserRole + 1,
    Role_ImageRequestId,
    Role_ImageFetchFinished,
    Role_ImageDimensions,  // width * height
    Role_ImageSize,
  };

  void Init(AlbumCoverFetcher* fetcher);

  QImage Exec(const QString& artist, const QString& album);

 protected:
  void keyPressEvent(QKeyEvent*);

 private slots:
  void Search();
  void SearchFinished(quint64 id, const CoverSearchResults& results);
  void ImageLoaded(quint64 id, const QImage& image);

  void CoverDoubleClicked(const QModelIndex& index);

 private:
  Ui_AlbumCoverSearcher* ui_;

  Application* app_;
  QStandardItemModel* model_;

  QIcon no_cover_icon_;
  AlbumCoverLoaderOptions options_;
  AlbumCoverFetcher* fetcher_;

  quint64 id_;
  QMap<quint64, QStandardItem*> cover_loading_tasks_;
};

#endif  // ALBUMCOVERSEARCHER_H
