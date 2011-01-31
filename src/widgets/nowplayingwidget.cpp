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

#include "nowplayingwidget.h"
#include "core/albumcoverloader.h"
#include "core/kittenloader.h"
#include "library/librarybackend.h"
#include "ui/albumcoverchoicecontroller.h"
#include "ui/iconloader.h"

#include <QFileDialog>
#include <QImageWriter>
#include <QMenu>
#include <QMovie>
#include <QPainter>
#include <QPaintEvent>
#include <QSettings>
#include <QSignalMapper>
#include <QTextDocument>
#include <QTimeLine>
#include <QtDebug>

const char* NowPlayingWidget::kSettingsGroup = "NowPlayingWidget";

const char* NowPlayingWidget::kHypnotoadPath = ":/hypnotoad.gif";

// Space between the cover and the details in small mode
const int NowPlayingWidget::kPadding = 2;

// Width of the transparent to black gradient above and below the text in large
// mode
const int NowPlayingWidget::kGradientHead = 40;
const int NowPlayingWidget::kGradientTail = 20;

// Maximum height of the cover in large mode, and offset between the
// bottom of the cover and bottom of the widget
const int NowPlayingWidget::kMaxCoverSize = 260;
const int NowPlayingWidget::kBottomOffset = 0;

// Border for large mode
const int NowPlayingWidget::kTopBorder = 4;


NowPlayingWidget::NowPlayingWidget(QWidget *parent)
  : QWidget(parent),
    album_cover_choice_controller_(new AlbumCoverChoiceController(this)),
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    kitten_loader_(NULL),
    mode_(SmallSongDetails),
    menu_(new QMenu(this)),
    save_dialog_(NULL),
    above_statusbar_action_(NULL),
    visible_(false),
    small_ideal_height_(0),
    cover_height_(0),
    show_hide_animation_(new QTimeLine(500, this)),
    fade_animation_(new QTimeLine(1000, this)),
    load_cover_id_(0),
    details_(new QTextDocument(this)),
    previous_track_opacity_(0.0),
    hypnotoad_(NULL),
    aww_(false)
{
  // Load settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  mode_ = Mode(s.value("mode", SmallSongDetails).toInt());

  // Context menu
  QActionGroup* mode_group = new QActionGroup(this);
  QSignalMapper* mode_mapper = new QSignalMapper(this);
  connect(mode_mapper, SIGNAL(mapped(int)), SLOT(SetMode(int)));
  CreateModeAction(SmallSongDetails, tr("Small album cover"), mode_group, mode_mapper);
  CreateModeAction(LargeSongDetails, tr("Large album cover"), mode_group, mode_mapper);

  menu_->addActions(mode_group->actions());
  menu_->addSeparator();

  QList<QAction*> actions = album_cover_choice_controller_->GetAllActions();

  connect(album_cover_choice_controller_->cover_from_file_action(),
          SIGNAL(triggered()), this, SLOT(LoadCoverFromFile()));
  connect(album_cover_choice_controller_->cover_from_url_action(),
          SIGNAL(triggered()), this, SLOT(LoadCoverFromURL()));
  connect(album_cover_choice_controller_->search_for_cover_action(),
          SIGNAL(triggered()), this, SLOT(SearchForCover()));
  connect(album_cover_choice_controller_->unset_cover_action(),
          SIGNAL(triggered()), this, SLOT(UnsetCover()));
  connect(album_cover_choice_controller_->show_cover_action(),
          SIGNAL(triggered()), this, SLOT(ShowCover()));
  connect(album_cover_choice_controller_->save_cover_action(),
          SIGNAL(triggered()), this, SLOT(SaveCover()));

  menu_->addActions(actions);
  menu_->addSeparator();
  above_statusbar_action_ = menu_->addAction(tr("Show above status bar"));

  above_statusbar_action_->setCheckable(true);
  connect(above_statusbar_action_, SIGNAL(toggled(bool)), SLOT(ShowAboveStatusBar(bool)));
  above_statusbar_action_->setChecked(s.value("above_status_bar", false).toBool());

  // Animations
  connect(show_hide_animation_, SIGNAL(frameChanged(int)), SLOT(SetHeight(int)));
  setMaximumHeight(0);

  connect(fade_animation_, SIGNAL(valueChanged(qreal)), SLOT(FadePreviousTrack(qreal)));
  fade_animation_->setDirection(QTimeLine::Backward); // 1.0 -> 0.0

  // Start loading the cover loader thread
  cover_loader_->Start();
  connect(cover_loader_, SIGNAL(Initialised()), SLOT(CoverLoaderInitialised()));
}

void NowPlayingWidget::CreateModeAction(Mode mode, const QString &text, QActionGroup *group, QSignalMapper* mapper) {
  QAction* action = new QAction(text, group);
  action->setCheckable(true);
  mapper->setMapping(action, mode);
  connect(action, SIGNAL(triggered()), mapper, SLOT(map()));

  if (mode == mode_)
    action->setChecked(true);
}

void NowPlayingWidget::set_ideal_height(int height) {
  small_ideal_height_ = height;
  UpdateHeight(aww_
      ? kitten_loader_->Worker().get()
      : cover_loader_->Worker().get());
}

QSize NowPlayingWidget::sizeHint() const {
  return QSize(cover_height_, total_height_);
}

void NowPlayingWidget::CoverLoaderInitialised() {
  BackgroundThread<AlbumCoverLoader>* loader =
      static_cast<BackgroundThread<AlbumCoverLoader>*>(sender());
  UpdateHeight(loader->Worker().get());
  loader->Worker()->SetPadOutputImage(true);
  connect(loader->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage,QImage)),
          SLOT(AlbumArtLoaded(quint64,QImage,QImage)));
}

void NowPlayingWidget::UpdateHeight(AlbumCoverLoader* loader) {
  switch (mode_) {
  case SmallSongDetails:
    cover_height_ = small_ideal_height_;
    total_height_ = small_ideal_height_;
    break;

  case LargeSongDetails:
    cover_height_ = qMin(kMaxCoverSize, width());
    total_height_ = kTopBorder + cover_height_ + kBottomOffset;
    break;
  }

  // Update the animation settings and resize the widget now if we're visible
  show_hide_animation_->setFrameRange(0, total_height_);
  if (visible_ && show_hide_animation_->state() != QTimeLine::Running)
    setMaximumHeight(total_height_);

  // Tell the cover loader what size we want the images in
  loader->SetDesiredHeight(cover_height_);
  loader->SetDefaultOutputImage(QImage(":nocover.png"));

  // Re-fetch the current image
  load_cover_id_ = loader->LoadImageAsync(metadata_);

  // Tell Qt we've changed size
  updateGeometry();
}

void NowPlayingWidget::NowPlaying(const Song& metadata) {
  if (visible_) {
    // Cache the current pixmap so we can fade between them
    previous_track_ = QPixmap(size());
    previous_track_.fill(palette().background().color());
    previous_track_opacity_ = 1.0;
    QPainter p(&previous_track_);
    DrawContents(&p);
    p.end();
  }

  metadata_ = metadata;
  cover_ = QPixmap();

  // Loads the cover too.
  UpdateHeight(aww_
      ? kitten_loader_->Worker().get()
      : cover_loader_->Worker().get());
  UpdateDetailsText();

  SetVisible(true);
  update();
}

void NowPlayingWidget::Stopped() {
  SetVisible(false);
}

void NowPlayingWidget::UpdateDetailsText() {
  QString html;

  switch (mode_) {
    case SmallSongDetails:
      details_->setTextWidth(-1);
      details_->setDefaultStyleSheet("");
      html += "<p>";
      break;

    case LargeSongDetails:
      details_->setTextWidth(cover_height_);
      details_->setDefaultStyleSheet("p {"
          "  font-size: small;"
          "  color: white;"
          "}");
      html += "<p align=center>";
      break;
  }

  // TODO: Make this configurable
  html += QString("<i>%1</i><br/>%2<br/>%3").arg(
      Qt::escape(metadata_.PrettyTitle()), Qt::escape(metadata_.artist()),
      Qt::escape(metadata_.album()));

  html += "</p>";
  details_->setHtml(html);
}

void NowPlayingWidget::AlbumArtLoaded(quint64 id, const QImage& scaled, const QImage& original) {
  if (id != load_cover_id_)
    return;

  cover_ = QPixmap::fromImage(scaled);
  original_ = original;
  update();

  // Were we waiting for this cover to load before we started fading?
  if (!previous_track_.isNull()) {
    fade_animation_->start();
  }
}

void NowPlayingWidget::SetHeight(int height) {
  setMaximumHeight(height);
}

void NowPlayingWidget::SetVisible(bool visible) {
  if (visible == visible_)
    return;
  visible_ = visible;

  show_hide_animation_->setDirection(visible ? QTimeLine::Forward : QTimeLine::Backward);
  show_hide_animation_->start();
}

void NowPlayingWidget::paintEvent(QPaintEvent *e) {
  QPainter p(this);

  DrawContents(&p);

  // Draw the previous track's image if we're fading
  if (!previous_track_.isNull()) {
    p.setOpacity(previous_track_opacity_);
    p.drawPixmap(0, 0, previous_track_);
  }
}

void NowPlayingWidget::DrawContents(QPainter *p) {
  switch (mode_) {
  case SmallSongDetails:
    if (hypnotoad_) {
      p->drawPixmap(0, 0, small_ideal_height_, small_ideal_height_, hypnotoad_->currentPixmap());
    } else {
      // Draw the cover
      p->drawPixmap(0, 0, small_ideal_height_, small_ideal_height_, cover_);
    }

    // Draw the details
    p->translate(small_ideal_height_ + kPadding, 0);
    details_->drawContents(p);
    p->translate(-small_ideal_height_ - kPadding, 0);
    break;

  case LargeSongDetails:
    const int total_size = qMin(kMaxCoverSize, width());
    const int x_offset = (width() - cover_height_) / 2;

    // Draw the black background
    p->fillRect(QRect(0, kTopBorder, width(), height() - kTopBorder), Qt::black);

    // Draw the cover
    if (hypnotoad_) {
      p->drawPixmap(x_offset, kTopBorder, total_size, total_size, hypnotoad_->currentPixmap());
    } else {
      p->drawPixmap(x_offset, kTopBorder, total_size, total_size, cover_);
    }

    // Work out how high the text is going to be
    const int text_height = details_->size().height();
    const int gradient_mid = height() - qMax(text_height, kBottomOffset);

    // Draw the black fade
    QLinearGradient gradient(0, gradient_mid - kGradientHead,
                             0, gradient_mid + kGradientTail);
    gradient.setColorAt(0, QColor(0, 0, 0, 0));
    gradient.setColorAt(1, QColor(0, 0, 0, 255));

    p->fillRect(0, gradient_mid - kGradientHead,
                width(), height() - (gradient_mid - kGradientHead), gradient);

    // Draw the text on top
    p->translate(x_offset, height() - text_height);
    details_->drawContents(p);
    p->translate(-x_offset, -height() + text_height);
    break;
  }
}

void NowPlayingWidget::FadePreviousTrack(qreal value) {
  previous_track_opacity_ = value;
  if (qFuzzyCompare(previous_track_opacity_, 0.0)) {
    previous_track_ = QPixmap();
  }

  update();
}

void NowPlayingWidget::SetMode(int mode) {
  mode_ = Mode(mode);
  UpdateHeight(aww_
      ? kitten_loader_->Worker().get()
      : cover_loader_->Worker().get());
  UpdateDetailsText();
  update();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("mode", mode_);
}

void NowPlayingWidget::resizeEvent(QResizeEvent* e) {
  if (visible_ && mode_ == LargeSongDetails && e->oldSize().width() != e->size().width()) {
    UpdateHeight(aww_
        ? kitten_loader_->Worker().get()
        : cover_loader_->Worker().get());
    UpdateDetailsText();
  }
}

void NowPlayingWidget::contextMenuEvent(QContextMenuEvent* e) {
#ifndef HAVE_LIBLASTFM
  album_cover_choice_controller_->cover_from_file_action()->setEnabled(false);
  album_cover_choice_controller_->search_for_cover_action()->setEnabled(false);
#endif

  const bool art_is_not_set =
     metadata_.art_manual() == AlbumCoverLoader::kManuallyUnsetCover
 || (metadata_.art_automatic().isEmpty() && metadata_.art_manual().isEmpty());

  album_cover_choice_controller_->unset_cover_action()->setEnabled(!art_is_not_set);
  album_cover_choice_controller_->show_cover_action()->setEnabled(!art_is_not_set);

  menu_->popup(mapToGlobal(e->pos()));
}

void NowPlayingWidget::ShowAboveStatusBar(bool above) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("above_status_bar", above);

  emit ShowAboveStatusBarChanged(above);
}

bool NowPlayingWidget::show_above_status_bar() const {
  return above_statusbar_action_->isChecked();
}

void NowPlayingWidget::AllHail(bool hypnotoad) {
  if (hypnotoad) {
    hypnotoad_ = new QMovie(kHypnotoadPath, QByteArray(), this);
    connect(hypnotoad_, SIGNAL(updated(const QRect&)), SLOT(update()));
    hypnotoad_->start();
    update();
  } else {
    delete hypnotoad_;
    hypnotoad_ = NULL;
    update();
  }
}

void NowPlayingWidget::EnableKittens(bool aww) {
  if (!kitten_loader_ && aww) {
    kitten_loader_ = new BackgroundThreadImplementation<AlbumCoverLoader, KittenLoader>(this);
    kitten_loader_->Start();
    connect(kitten_loader_, SIGNAL(Initialised()), SLOT(CoverLoaderInitialised()));
  } else if (aww) {
    NowPlaying(metadata_);
  }

  aww_ = aww;
}

void NowPlayingWidget::LoadCoverFromFile() {
  QString cover = album_cover_choice_controller_->LoadCoverFromFile(&metadata_);

  if(!cover.isEmpty())
    NowPlaying(metadata_);
}

void NowPlayingWidget::LoadCoverFromURL() {
  QString cover = album_cover_choice_controller_->LoadCoverFromURL(&metadata_);

  if(!cover.isEmpty())
    NowPlaying(metadata_);
}

void NowPlayingWidget::SearchForCover() {
  QString cover = album_cover_choice_controller_->SearchForCover(&metadata_);

  if(!cover.isEmpty())
    NowPlaying(metadata_);
}

void NowPlayingWidget::SaveCover() {
  if (!save_dialog_) {
    save_dialog_ = new QFileDialog(
        this,
        tr("Save Album Cover"),
        QDir::home().absolutePath(),
        tr("Images (*.jpg)"));
    save_dialog_->setAcceptMode(QFileDialog::AcceptSave);
  }
  save_dialog_->selectFile(metadata_.album() + ".jpg");

  if (!save_dialog_->exec()) {
    return;
  }

  QStringList filenames = save_dialog_->selectedFiles();
  QString save_filename = filenames[0];

  QString extension = save_filename.right(4);
  if (!extension.startsWith('.') ||
      !QImageWriter::supportedImageFormats().contains(extension.right(3).toUtf8())) {
    save_filename.append(".jpg");
  }
  original_.save(save_filename);
}

void NowPlayingWidget::UnsetCover() {
  album_cover_choice_controller_->UnsetCover(&metadata_);
  NowPlaying(metadata_);
}

void NowPlayingWidget::ShowCover() {
  album_cover_choice_controller_->ShowCover(metadata_);
}

void NowPlayingWidget::SetLibraryBackend(LibraryBackend* backend) {
  album_cover_choice_controller_->SetLibrary(backend);
}
