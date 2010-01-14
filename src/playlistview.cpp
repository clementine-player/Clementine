#include "playlistview.h"
#include "playlist.h"
#include "playlistheader.h"
#include "radioloadingindicator.h"

#include <QPainter>
#include <QHeaderView>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>

#include <math.h>

const char* PlaylistView::kSettingsGroup = "Playlist";
const int PlaylistView::kGlowIntensitySteps = 32;


PlaylistDelegateBase::PlaylistDelegateBase(QTreeView* view)
  : QStyledItemDelegate(view),
    view_(view)
{
}

QString PlaylistDelegateBase::displayText(const QVariant& value, const QLocale&) const {
  switch (value.type()) {
    case QVariant::Int: {
      int v = value.toInt();
      if (v <= 0)
        return QString::null;
      return QString::number(v);
    }

    case QVariant::Double: {
      double v = value.toDouble();
      if (v <= 0)
        return QString::null;
      return QString::number(v);
    }

    default:
      return value.toString();
  }
}

void PlaylistDelegateBase::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QStyledItemDelegate::paint(painter, Adjusted(option, index), index);

  if (view_->header()->logicalIndexAt(QPoint(0,0)) == index.column()) {
    if (index.data(Playlist::Role_StopAfter).toBool()) {
      QColor color(Qt::white);
      if (!index.data(Playlist::Role_IsCurrent).toBool() &&
          !(option.state & QStyle::State_Selected)) {
        color = option.palette.color(QPalette::Highlight);
      }

      const int kStopSize = 10;
      const int kStopBorder = 2;

      QRect stop_rect(option.rect);
      stop_rect.setLeft(stop_rect.right() - kStopSize - kStopBorder);
      stop_rect.setWidth(kStopSize);
      stop_rect.moveTop(stop_rect.top() + (stop_rect.height() - kStopSize) / 2);
      stop_rect.setHeight(kStopSize);

      painter->setOpacity(0.65);
      painter->fillRect(stop_rect, color);
      painter->setOpacity(1.0);
    }
  }
}

QStyleOptionViewItemV4 PlaylistDelegateBase::Adjusted(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if (view_->header()->logicalIndexAt(QPoint(0,0)) != index.column())
    return option;

  QStyleOptionViewItemV4 ret(option);

  if (index.data(Playlist::Role_IsCurrent).toBool()) {
    // Move the text in a bit on the first column for the song that's currently
    // playing
    ret.rect.setLeft(ret.rect.left() + 20);
  }

  return ret;
}


QString LengthItemDelegate::displayText(const QVariant& value, const QLocale&) const {
  bool ok = false;
  int seconds = value.toInt(&ok);
  QString ret;

  if (ok && seconds > 0) {
    int hours = seconds / (60*60);
    int minutes = (seconds / 60) % 60;
    seconds %= 60;

    if (hours)
      ret.sprintf("%d:%02d:%02d", hours, minutes, seconds);
    else
      ret.sprintf("%d:%02d", minutes, seconds);
  }
  return ret;
}


QString SizeItemDelegate::displayText(const QVariant& value, const QLocale&) const {
  bool ok = false;
  int bytes = value.toInt(&ok);
  QString ret;

  if (ok && bytes > 0) {
    if (bytes <= 1024)
      ret.sprintf("%d bytes", bytes);
    else if (bytes <= 1024*1024)
      ret.sprintf("%.1f KB", float(bytes) / 1024);
    else if (bytes <= 1024*1024*1024)
      ret.sprintf("%.1f MB", float(bytes) / (1024*1024));
    else
      ret.sprintf("%.1f GB", float(bytes) / (1024*1024*1024));
  }
  return ret;
}


PlaylistView::PlaylistView(QWidget *parent)
  : QTreeView(parent),
    glow_enabled_(false),
    glow_timer_(new QTimer(this)),
    glow_intensity_step_(0),
    row_height_(-1),
    currenttrack_play_(":currenttrack_play.png"),
    currenttrack_pause_(":currenttrack_pause.png"),
    menu_(new QMenu(this)),
    radio_loading_(new RadioLoadingIndicator(this))
{
  setItemDelegate(new PlaylistDelegateBase(this));
  setItemDelegateForColumn(Playlist::Column_Length, new LengthItemDelegate(this));
  setItemDelegateForColumn(Playlist::Column_Filesize, new SizeItemDelegate(this));

  setHeader(new PlaylistHeader(Qt::Horizontal, this));
  header()->setMovable(true);

  connect(header(), SIGNAL(sectionResized(int,int,int)), SLOT(SaveGeometry()));
  connect(header(), SIGNAL(sectionMoved(int,int,int)), SLOT(SaveGeometry()));

  glow_timer_->setInterval(1500 / kGlowIntensitySteps);
  connect(glow_timer_, SIGNAL(timeout()), SLOT(GlowIntensityChanged()));

  menu_->addAction(QIcon(":media-playback-stop.png"), "Stop playing after track",
                   this, SLOT(StopAfter()));

  radio_loading_->hide();
}

void PlaylistView::setModel(QAbstractItemModel *model) {
  QTreeView::setModel(model);
  LoadGeometry();
}

void PlaylistView::LoadGeometry() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  if (!header()->restoreState(settings.value("state").toByteArray())) {
    header()->hideSection(Playlist::Column_Disc);
    header()->hideSection(Playlist::Column_Year);
    header()->hideSection(Playlist::Column_Genre);
    header()->hideSection(Playlist::Column_BPM);
    header()->hideSection(Playlist::Column_Bitrate);
    header()->hideSection(Playlist::Column_Samplerate);
    header()->hideSection(Playlist::Column_Filename);
    header()->hideSection(Playlist::Column_Filesize);
  }
}

void PlaylistView::SaveGeometry() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("state", header()->saveState());
}

void PlaylistView::ReloadBarPixmaps() {
  currenttrack_bar_left_ = LoadBarPixmap(":currenttrack_bar_left.png");
  currenttrack_bar_mid_ = LoadBarPixmap(":currenttrack_bar_mid.png");
  currenttrack_bar_right_ = LoadBarPixmap(":currenttrack_bar_right.png");
}

QList<QPixmap> PlaylistView::LoadBarPixmap(const QString& filename) {
  QImage image(filename);
  image = image.scaledToHeight(row_height_, Qt::SmoothTransformation);

  // Colour the bar with the palette colour
  QPainter p(&image);
  p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
  p.setOpacity(0.4);
  p.fillRect(image.rect(), palette().color(QPalette::Highlight).lighter(125));
  p.end();

  // Animation steps
  QList<QPixmap> ret;
  for(int i=0 ; i<kGlowIntensitySteps ; ++i) {
    QImage step(image.copy());
    p.begin(&step);
    p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    p.setOpacity(0.4 - 0.6 * sin(float(i)/kGlowIntensitySteps * (M_PI/2)));
    p.fillRect(step.rect(), Qt::white);
    p.end();
    ret << QPixmap::fromImage(step);
  }

  return ret;
}

void PlaylistView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QStyleOptionViewItemV4 opt(option);

  bool is_current = index.data(Playlist::Role_IsCurrent).toBool();
  bool is_paused = index.data(Playlist::Role_IsPaused).toBool();

  if (is_current) {
    const_cast<PlaylistView*>(this)->last_current_item_ = index;
    const_cast<PlaylistView*>(this)->last_glow_rect_ = opt.rect;

    int step = glow_intensity_step_;
    if (step >= kGlowIntensitySteps)
      step = 2*(kGlowIntensitySteps-1) - step + 1;

    int row_height = opt.rect.height();
    if (row_height != row_height_) {
      // Recreate the pixmaps if the height changed since last time
      const_cast<PlaylistView*>(this)->row_height_ = row_height;
      const_cast<PlaylistView*>(this)->ReloadBarPixmaps();
    }

    QRect middle(opt.rect);
    middle.setLeft(middle.left() + currenttrack_bar_left_[0].width());
    middle.setRight(middle.right() - currenttrack_bar_right_[0].width());

    // Selection
    if (selectionModel()->isSelected(index))
      painter->fillRect(opt.rect, opt.palette.color(QPalette::Highlight));

    // Draw the bar
    painter->drawPixmap(opt.rect.topLeft(), currenttrack_bar_left_[step]);
    painter->drawPixmap(opt.rect.topRight() - currenttrack_bar_right_[0].rect().topRight(), currenttrack_bar_right_[step]);
    painter->drawPixmap(middle, currenttrack_bar_mid_[step]);

    // Draw the play icon
    QPoint play_pos(currenttrack_bar_left_[0].width() / 3 * 2,
                    (row_height - currenttrack_play_.height()) / 2);
    painter->drawPixmap(opt.rect.topLeft() + play_pos,
                        is_paused ? currenttrack_pause_ : currenttrack_play_);

    // Set the font
    opt.palette.setColor(QPalette::Text, Qt::white);
    opt.palette.setColor(QPalette::HighlightedText, Qt::white);
    opt.palette.setColor(QPalette::Highlight, Qt::transparent);
    opt.font.setItalic(true);
    opt.decorationSize = QSize(20,20);
  }

  QTreeView::drawRow(painter, opt, index);
}

void PlaylistView::GlowIntensityChanged() {
  glow_intensity_step_ = (glow_intensity_step_ + 1) % (kGlowIntensitySteps * 2);

  viewport()->update(last_glow_rect_);
}

void PlaylistView::StopGlowing() {
  glow_enabled_ = false;
  glow_timer_->stop();
  glow_intensity_step_ = kGlowIntensitySteps;
}

void PlaylistView::StartGlowing() {
  glow_enabled_ = true;
  if (isVisible())
    glow_timer_->start();
}

void PlaylistView::hideEvent(QHideEvent*) {
  glow_timer_->stop();
}

void PlaylistView::showEvent(QShowEvent*) {
  if (glow_enabled_)
    glow_timer_->start();
}

bool CompareSelectionRanges(const QItemSelectionRange& a, const QItemSelectionRange& b) {
  return b.bottom() < a.bottom();
}

void PlaylistView::keyPressEvent(QKeyEvent* event) {
  if (!model()) {
    QTreeView::keyPressEvent(event);
    return;
  }

  if (event->matches(QKeySequence::Delete) ||
      event->key() == Qt::Key_Backspace) {
    QItemSelection selection(selectionModel()->selection());

    // Sort the selection so we remove the items at the *bottom* first, ensuring
    // we don't have to mess around with changing row numbers
    qSort(selection.begin(), selection.end(), CompareSelectionRanges);

    foreach (const QItemSelectionRange& range, selection) {
      model()->removeRows(range.top(), range.height(), range.parent());
    }

    // Select the new current item
    if (currentIndex().isValid())
      selectionModel()->select(
          QItemSelection(currentIndex().sibling(currentIndex().row(), 0),
                         currentIndex().sibling(currentIndex().row(), model()->columnCount()-1)),
          QItemSelectionModel::Select);

    event->accept();
  } else if (event->key() == Qt::Key_Enter ||
             event->key() == Qt::Key_Return ||
             event->key() == Qt::Key_Space) {
    if (currentIndex().isValid())
      emit PlayPauseItem(currentIndex());
    event->accept();
  } else {
    QTreeView::keyPressEvent(event);
  }
}

void PlaylistView::contextMenuEvent(QContextMenuEvent* e) {
  menu_index_ = indexAt(e->pos());
  if (!menu_index_.isValid())
    return;

  menu_->popup(e->globalPos());
  e->accept();
}

void PlaylistView::StopAfter() {
  Playlist* playlist = qobject_cast<Playlist*>(model());
  Q_ASSERT(playlist);

  playlist->StopAfter(menu_index_.row());
}

void PlaylistView::resizeEvent(QResizeEvent *event) {
  const QPoint kPadding(5,5);

  QPoint pos(viewport()->mapTo(this, viewport()->rect().bottomRight()) -
             kPadding - QPoint(radio_loading_->sizeHint().width(),
                               radio_loading_->sizeHint().height()));

  radio_loading_->move(pos);
  radio_loading_->resize(radio_loading_->sizeHint());

  QTreeView::resizeEvent(event);
}

void PlaylistView::StartRadioLoading() {
  radio_loading_->show();
}

void PlaylistView::StopRadioLoading() {
  radio_loading_->hide();
}
