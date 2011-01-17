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

#ifndef EDITTAGDIALOG_H
#define EDITTAGDIALOG_H

#include <QDialog>
#include <QModelIndexList>

#include "config.h"
#include "core/backgroundthread.h"
#include "core/song.h"
#ifdef HAVE_LIBTUNEPIMP
# include "core/tagfetcher.h"
#endif
#include "playlist/playlistitem.h"
#include "widgets/lineedit.h"

#ifdef HAVE_LIBLASTFM
  class AlbumCoverFetcher;
  class AlbumCoverSearcher;
#endif

class AlbumCoverLoader;
class LibraryBackend;
class Ui_EditTagDialog;

class QAbstractButton;
class QItemSelection;
class QLabel;
class QPushButton;

class EditTagDialog : public QDialog {
  Q_OBJECT

public:
  EditTagDialog(QWidget* parent = 0);
  ~EditTagDialog();

  static const char* kHintText;
  static const char* kTagFetchText;
  static const char* kTagFetchOnLoadText;

  void SetSongs(const SongList& songs, const PlaylistItemList& items = PlaylistItemList());
  void SetTagCompleter(LibraryBackend* backend);

  PlaylistItemList playlist_items() const { return playlist_items_; }

  void accept();

signals:
  void Error(const QString& message);

protected:
  bool eventFilter(QObject* o, QEvent* e);
  void showEvent(QShowEvent*);

private slots:
  void SetSongsFinished();
  void AcceptFinished();

  void SelectionChanged();
  void FieldValueEdited();
  void ResetField();
  void ButtonClicked(QAbstractButton* button);
  void SongRated(float rating);
  void ResetPlayCounts();
  void FetchTag();
  void FetchTagFinished(const QString& filename, const SongList& songs_guessed);

  void ArtLoaded(quint64 id, const QImage& image);

  void LoadCoverFromFile();
  void SearchCover();
  void UnsetCover();
  void ZoomCover();

  void PreviousSong();
  void NextSong();

private:
  struct Data {
    Data(const Song& song = Song()) : original_(song), current_(song) {}

    static QVariant value(const Song& song, const QString& id);
    QVariant original_value(const QString& id) const { return value(original_, id); }
    QVariant current_value(const QString& id) const { return value(current_, id); }

    void set_value(const QString& id, const QVariant& value);

    Song original_;
    Song current_;
  };

  struct FieldData {
    FieldData(QLabel* label = NULL, QWidget* editor = NULL,
              const QString& id = QString())
      : label_(label), editor_(editor), id_(id) {}

    QLabel* label_;
    QWidget* editor_;
    QString id_;
  };

  bool DoesValueVary(const QModelIndexList& sel, const QString& id) const;
  bool IsValueModified(const QModelIndexList& sel, const QString& id) const;

  void InitFieldValue(const FieldData& field, const QModelIndexList& sel);
  void UpdateFieldValue(const FieldData& field, const QModelIndexList& sel);
  void ResetFieldValue(const FieldData& field, const QModelIndexList& sel);

  void UpdateSummaryTab(const Song& song);
  void UpdateStatisticsTab(const Song& song);

  void SetAlbumArt(const QString& path);

  bool SetLoading(const QString& message);

  // Called by QtConcurrentRun
  QList<Data> LoadData(const SongList& songs) const;
  void SaveData(const QList<Data>& data);

private:
  Ui_EditTagDialog* ui_;
  LibraryBackend* backend_;

  bool loading_;

  PlaylistItemList playlist_items_;
  QList<Data> data_;
  QList<FieldData> fields_;

  bool ignore_edits_;

#ifdef HAVE_LIBTUNEPIMP
  TagFetcher* tag_fetcher_;
#endif

#ifdef HAVE_LIBLASTFM
  AlbumCoverSearcher* cover_searcher_;
  AlbumCoverFetcher* cover_fetcher_;
#endif

  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  quint64 cover_art_id_;
  bool cover_art_is_set_;

  QMenu* cover_menu_;
  QAction* choose_cover_;
  QAction* download_cover_;
  QAction* unset_cover_;
  QAction* show_cover_;

  QPushButton* previous_button_;
  QPushButton* next_button_;
};

#endif // EDITTAGDIALOG_H
