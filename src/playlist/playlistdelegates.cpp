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

#include "playlistdelegates.h"
#include "queue.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "widgets/trackslider.h"
#include "ui/iconloader.h"

#include <QDateTime>
#include <QDir>
#include <QLineEdit>
#include <QPainter>
#include <QToolTip>
#include <QWhatsThis>
#include <QHelpEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QLinearGradient>

const int   QueuedItemDelegate::kQueueBoxBorder = 1;
const int   QueuedItemDelegate::kQueueBoxCornerRadius = 3;
const int   QueuedItemDelegate::kQueueBoxLength = 30;
const QRgb  QueuedItemDelegate::kQueueBoxGradientColor1 = qRgb(102, 150, 227);
const QRgb  QueuedItemDelegate::kQueueBoxGradientColor2 = qRgb(77, 121, 200);
const int   QueuedItemDelegate::kQueueOpacitySteps = 10;
const float QueuedItemDelegate::kQueueOpacityLowerBound = 0.4;

const int   PlaylistDelegateBase::kMinHeight = 19;

const int   RatingItemDelegate::kStarCount;
const int   RatingItemDelegate::kStarSize;

QueuedItemDelegate::QueuedItemDelegate(QObject *parent, int indicator_column)
  : QStyledItemDelegate(parent),
    indicator_column_(indicator_column)
{
}

void QueuedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  QStyledItemDelegate::paint(painter, option, index);

  if (index.column() == indicator_column_) {
    bool ok = false;
    const int queue_pos = index.data(Playlist::Role_QueuePosition).toInt(&ok);
    if (ok && queue_pos != -1) {
      float opacity = kQueueOpacitySteps - qMin(kQueueOpacitySteps, queue_pos);
      opacity /= kQueueOpacitySteps;
      opacity *= 1.0 - kQueueOpacityLowerBound;
      opacity += kQueueOpacityLowerBound;
      painter->setOpacity(opacity);

      DrawBox(painter, option.rect, option.font, QString::number(queue_pos+1),
              kQueueBoxLength);

      painter->setOpacity(1.0);
    }
  }
}

void QueuedItemDelegate::DrawBox(
    QPainter* painter, const QRect& line_rect, const QFont& font,
    const QString& text, int width) const {
  QFont smaller = font;
  smaller.setPointSize(smaller.pointSize() - 1);
  smaller.setBold(true);

  if (width == -1)
    width = QFontMetrics(font).width(text + "  ");

  QRect rect(line_rect);
  rect.setLeft(rect.right() - width - kQueueBoxBorder);
  rect.setWidth(width);
  rect.setTop(rect.top() + kQueueBoxBorder);
  rect.setBottom(rect.bottom() - kQueueBoxBorder - 1);

  QRect text_rect(rect);
  text_rect.setBottom(text_rect.bottom() + 1);

  QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
  gradient.setColorAt(0.0, kQueueBoxGradientColor1);
  gradient.setColorAt(1.0, kQueueBoxGradientColor2);

  // Turn on antialiasing
  painter->setRenderHint(QPainter::Antialiasing);

  // Draw the box
  painter->setPen(QPen(Qt::white, 1));
  painter->setBrush(gradient);
  painter->drawRoundedRect(rect, kQueueBoxCornerRadius, kQueueBoxCornerRadius);

  // Draw the text
  painter->setFont(smaller);
  painter->drawText(rect, Qt::AlignCenter, text);
}

int QueuedItemDelegate::queue_indicator_size(const QModelIndex& index) const {
  if (index.column() == indicator_column_) {
    const int queue_pos = index.data(Playlist::Role_QueuePosition).toInt();
    if (queue_pos != -1) {
      return kQueueBoxLength + kQueueBoxBorder*2;
    }
  }
  return 0;
}


PlaylistDelegateBase::PlaylistDelegateBase(QObject* parent, const QString& suffix)
  : QueuedItemDelegate(parent),
    view_(qobject_cast<QTreeView*>(parent)),
    suffix_(suffix)
{
}

QString PlaylistDelegateBase::displayText(const QVariant& value, const QLocale&) const {
  QString text;

  switch (value.type()) {
    case QVariant::Int: {
      int v = value.toInt();
      if (v > 0)
        text = QString::number(v);
      break;
    }

    case QMetaType::Float:
    case QVariant::Double: {
      double v = value.toDouble();
      if (v > 0)
        text = QString::number(v);
      break;
    }

    default:
      text = value.toString();
      break;
  }

  if (!text.isNull() && !suffix_.isNull())
    text += " " + suffix_;
  return text;
}

QSize PlaylistDelegateBase::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
  QSize size = QueuedItemDelegate::sizeHint(option, index);
  if (size.height() < kMinHeight)
    size.setHeight(kMinHeight);
  return size;
}

void PlaylistDelegateBase::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QueuedItemDelegate::paint(painter, Adjusted(option, index), index);

  // Stop after indicator
  if (index.column() == Playlist::Column_Title) {
    if (index.data(Playlist::Role_StopAfter).toBool()) {
      QRect rect(option.rect);
      rect.setRight(rect.right() - queue_indicator_size(index));

      DrawBox(painter, rect, option.font, tr("stop"));
    }
  }
}

QStyleOptionViewItemV4 PlaylistDelegateBase::Adjusted(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if (!view_)
    return option;

  QPoint top_left(-view_->horizontalScrollBar()->value(),
                  -view_->verticalScrollBar()->value());

  if (view_->header()->logicalIndexAt(top_left) != index.column())
    return option;

  QStyleOptionViewItemV4 ret(option);

  if (index.data(Playlist::Role_IsCurrent).toBool()) {
    // Move the text in a bit on the first column for the song that's currently
    // playing
    ret.rect.setLeft(ret.rect.left() + 20);
  }

  return ret;
}

bool PlaylistDelegateBase::helpEvent(QHelpEvent *event, QAbstractItemView *view,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) {
  // This function is copied from QAbstractItemDelegate, and changed to show
  // displayText() in the tooltip, rather than the index's naked
  // Qt::ToolTipRole text.

  Q_UNUSED(option);

  if (!event || !view)
    return false;

  QHelpEvent *he = static_cast<QHelpEvent*>(event);
  QString text = displayText(index.data(), QLocale::system());

  if (text.isEmpty() || !he)
    return false;

  switch (event->type()) {
    case QEvent::ToolTip:
      QToolTip::showText(he->globalPos(), text, view);
      return true;

    case QEvent::QueryWhatsThis:
      return true;

    case QEvent::WhatsThis:
      QWhatsThis::showText(he->globalPos(), text, view);
      return true;

    default:
      break;
  }
  return false;
}


QString LengthItemDelegate::displayText(const QVariant& value, const QLocale&) const {
  bool ok = false;
  int seconds = value.toInt(&ok);

  if (ok && seconds > 0)
    return Utilities::PrettyTime(seconds);
  return QString::null;
}


QString SizeItemDelegate::displayText(const QVariant& value, const QLocale&) const {
  bool ok = false;
  int bytes = value.toInt(&ok);

  if (ok)
    return Utilities::PrettySize(bytes);
  return QString();
}

QString DateItemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  bool ok = false;
  int time = value.toInt(&ok);

  if (!ok || time == -1)
    return QString::null;

  return QDateTime::fromTime_t(time).toString(
      QLocale::system().dateTimeFormat(QLocale::ShortFormat));
}

QString LastPlayedItemDelegate::displayText(const QVariant& value, const QLocale& locale) const {
  bool ok = false;
  const int time = value.toInt(&ok);

  if (!ok || time == -1)
    return tr("Never");

  return Utilities::Ago(time, locale);
}

QString FileTypeItemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  bool ok = false;
  Song::FileType type = Song::FileType(value.toInt(&ok));

  if (!ok)
    return tr("Unknown");

  return Song::TextForFiletype(type);
}

QWidget* TextItemDelegate::createEditor(
    QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  return new QLineEdit(parent);
}

RatingItemDelegate::RatingItemDelegate(QObject* parent)
  : PlaylistDelegateBase(parent)
{
  // Load the base pixmaps
  QPixmap on(":/star-on.png");
  QPixmap off(":/star-off.png");

  // Generate the 10 states, better to do it now than on the fly
  for (int i=0 ; i<kStarCount*2+1 ; ++i) {
    const float rating = float(i) / 2.0;

    // Clear the pixmap
    stars_[i] = QPixmap(kStarSize * kStarCount, kStarSize);
    stars_[i].fill(Qt::transparent);
    QPainter p(&stars_[i]);

    // Draw the stars
    int x = 0;
    for (int i=0 ; i<kStarCount ; ++i, x+=kStarSize) {
      const QRect rect(x, 0, kStarSize, kStarSize);

      if (rating - 0.25 <= i) {
        // Totally empty
        p.drawPixmap(rect, off);
      } else if (rating - 0.75 <= i) {
        // Half full
        const QRect target_left(rect.x(), rect.y(), kStarSize/2, kStarSize);
        const QRect target_right(rect.x() + kStarSize/2, rect.y(), kStarSize/2, kStarSize);
        const QRect source_left(0, 0, kStarSize/2, kStarSize);
        const QRect source_right(kStarSize/2, 0, kStarSize/2, kStarSize);
        p.drawPixmap(target_left, on, source_left);
        p.drawPixmap(target_right, off, source_right);
      } else {
        // Totally full
        p.drawPixmap(rect, on);
      }
    }
  }
}

QRect RatingItemDelegate::ContentRect(const QRect& total) {
  const int width = total.height() * kStarCount;
  const int x = total.x() + (total.width() - width) / 2;

  return QRect(x, total.y(), width, total.height());
}

double RatingItemDelegate::RatingForPos(const QPoint& pos, const QRect& total_rect) {
  const QRect contents = ContentRect(total_rect);
  const double raw = double(pos.x() - contents.left()) / contents.width();

  // Round to the nearest 0.1
  return double(int(raw * kStarCount * 2 + 0.5)) / (kStarCount * 2);
}

void RatingItemDelegate::paint(
    QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  // Draw the background
  const QStyleOptionViewItemV3* vopt =
      qstyleoption_cast<const QStyleOptionViewItemV3*>(&option);
  vopt->widget->style()->drawPrimitive(
      QStyle::PE_PanelItemViewItem, vopt, painter, vopt->widget);

  // Don't draw anything else if the user can't set the rating of this item
  if (!index.data(Playlist::Role_CanSetRating).toBool())
    return;

  QSize size(qMin(kStarSize*kStarCount, option.rect.width()),
             qMin(kStarSize, option.rect.height()));
  QPoint pos(option.rect.center() - QPoint(size.width() / 2, size.height() / 2));

  const bool hover = mouse_over_index_ == index;
  const double rating = hover ? double(mouse_over_pos_.x() - pos.x()) / kStarSize
                              : index.data().toDouble() * kStarCount;

  // Draw the stars
  const int star = qBound(0, int(rating*2.0 + 0.5), kStarCount*2);
  painter->drawPixmap(QRect(pos, size), stars_[star], QRect(QPoint(0,0), size));
}

QSize RatingItemDelegate::sizeHint(
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QSize size = PlaylistDelegateBase::sizeHint(option, index);
  size.setWidth(size.height() * kStarCount);
  return size;
}

QString RatingItemDelegate::displayText(
    const QVariant& value, const QLocale&) const {
  if (value.isNull() || value.toDouble() <= 0)
    return QString();

  // Round to the nearest 0.5
  const double rating = float(int(value.toDouble() * kStarCount * 2 + 0.5)) / 2;

  return QString::number(rating, 'f', 1);
}

TagCompletionModel::TagCompletionModel(LibraryBackend* backend, Playlist::Column column) :
  QStringListModel() {

  switch(column) {
    case Playlist::Column_Artist: {
      setStringList(backend->GetAllArtists());
      break;
    }
    case Playlist::Column_Album: {
      QStringList album_names;
      LibraryBackend::AlbumList albums = backend->GetAllAlbums();
      foreach(const LibraryBackend::Album& album, albums)
        album_names << album.album_name;
      setStringList(album_names);
      break;
    }
    case Playlist::Column_AlbumArtist: {
      // TODO: get all albumartists?
      break;
    }
    default:
      break;
  }
}

TagCompleter::TagCompleter(LibraryBackend* backend, Playlist::Column column, QLineEdit* editor) :
  QCompleter(editor) {

  setModel(new TagCompletionModel(backend, column));
  setCaseSensitivity(Qt::CaseInsensitive);
  editor->setCompleter(this);
}

QWidget* TagCompletionItemDelegate::createEditor(
    QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {

  QLineEdit* editor = new QLineEdit(parent);
  new TagCompleter(backend_, column_, editor);

  return editor;
}

QString NativeSeparatorsDelegate::displayText(const QVariant& value, const QLocale&) const {
  QString str = value.toString();
  if (str.contains("://"))
    return str;
  return QDir::toNativeSeparators(str);
}
