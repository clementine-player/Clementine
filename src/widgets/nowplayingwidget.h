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

#ifndef NOWPLAYINGWIDGET_H
#define NOWPLAYINGWIDGET_H

#include <memory>

#include <QWidget>

#include "core/song.h"
#include "covers/albumcoverloaderoptions.h"

class AlbumCoverChoiceController;
class Application;
class FullscreenHypnotoad;
class KittenLoader;

class QAction;
class QActionGroup;
class QMenu;
class QMovie;
class QSignalMapper;
class QTextDocument;
class QTimeLine;

class NowPlayingWidget : public QWidget {
  Q_OBJECT

 public:
  NowPlayingWidget(QWidget* parent = 0);
  ~NowPlayingWidget();

  static const char* kSettingsGroup;
  static const int kPadding;
  static const int kGradientHead;
  static const int kGradientTail;
  static const int kMaxCoverSize;
  static const int kBottomOffset;
  static const int kTopBorder;

  // Values are saved in QSettings
  enum Mode { SmallSongDetails = 0, LargeSongDetails = 1, };

  void SetApplication(Application* app);

  void set_ideal_height(int height);
  bool show_above_status_bar() const;

  QSize sizeHint() const;

signals:
  void ShowAboveStatusBarChanged(bool above);

 public slots:
  void Stopped();
  void AllHail(bool hypnotoad);
  void EnableKittens(bool aww);

 protected:
  void paintEvent(QPaintEvent* e);
  void resizeEvent(QResizeEvent*);
  void contextMenuEvent(QContextMenuEvent* e);
  void dragEnterEvent(QDragEnterEvent* e);
  void dropEvent(QDropEvent* e);

 private slots:
  void SetMode(int mode);
  void ShowAboveStatusBar(bool above);

  void AlbumArtLoaded(const Song& metadata, const QString& uri,
                      const QImage& image);
  void KittenLoaded(quint64 id, const QImage& image);

  void SetVisible(bool visible);
  void SetHeight(int height);

  void FadePreviousTrack(qreal value);

  void LoadCoverFromFile();
  void SaveCoverToFile();
  void LoadCoverFromURL();
  void SearchForCover();
  void UnsetCover();
  void ShowCover();
  void SearchCoverAutomatically();

  void Bask();

  void AutomaticCoverSearchDone();

 private:
  void CreateModeAction(Mode mode, const QString& text, QActionGroup* group,
                        QSignalMapper* mapper);
  void UpdateDetailsText();
  void UpdateHeight();
  void DrawContents(QPainter* p);
  void SetImage(const QImage& image);
  void ScaleCover();
  bool GetCoverAutomatically();

 private:
  Application* app_;
  AlbumCoverChoiceController* album_cover_choice_controller_;

  Mode mode_;

  QMenu* menu_;

  QAction* above_statusbar_action_;

  bool visible_;
  int small_ideal_height_;
  AlbumCoverLoaderOptions cover_loader_options_;
  int total_height_;
  QTimeLine* show_hide_animation_;
  QTimeLine* fade_animation_;

  // Information about the current track
  Song metadata_;
  QPixmap cover_;
  // A copy of the original, unscaled album cover.
  QImage original_;
  QTextDocument* details_;

  // Holds the last track while we're fading to the new track
  QPixmap previous_track_;
  qreal previous_track_opacity_;

  static const char* kHypnotoadPath;
  QAction* bask_in_his_glory_action_;
  std::unique_ptr<QMovie> hypnotoad_;
  std::unique_ptr<FullscreenHypnotoad> big_hypnotoad_;

  std::unique_ptr<QMovie> spinner_animation_;
  bool downloading_covers_;

  bool aww_;
  KittenLoader* kittens_;
  quint64 pending_kitten_;
};

#endif  // NOWPLAYINGWIDGET_H
