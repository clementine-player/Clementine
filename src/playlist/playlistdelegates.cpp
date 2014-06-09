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

#include "playlistdelegates.h"

#include <QDateTime>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QHelpEvent>
#include <QLinearGradient>
#include <QLineEdit>
#include <QPainter>
#include <QScrollBar>
#include <QTextDocument>
#include <QToolTip>
#include <QWhatsThis>
#include <QtConcurrentRun>

#include "queue.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "widgets/trackslider.h"
#include "ui/iconloader.h"

#ifdef Q_OS_DARWIN
#include "core/mac_utilities.h"
#endif  // Q_OS_DARWIN

const int QueuedItemDelegate::kQueueBoxBorder = 1;
const int QueuedItemDelegate::kQueueBoxCornerRadius = 3;
const int QueuedItemDelegate::kQueueBoxLength = 30;
const QRgb QueuedItemDelegate::kQueueBoxGradientColor1 = qRgb(102, 150, 227);
const QRgb QueuedItemDelegate::kQueueBoxGradientColor2 = qRgb(77, 121, 200);
const int QueuedItemDelegate::kQueueOpacitySteps = 10;
const float QueuedItemDelegate::kQueueOpacityLowerBound = 0.4;

const int PlaylistDelegateBase::kMinHeight = 19;

QueuedItemDelegate::QueuedItemDelegate(QObject* parent, int indicator_column)
    : QStyledItemDelegate(parent), indicator_column_(indicator_column) {}

void QueuedItemDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
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

      DrawBox(painter, option.rect, option.font, QString::number(queue_pos + 1),
              kQueueBoxLength);

      painter->setOpacity(1.0);
    }
  }
}

void QueuedItemDelegate::DrawBox(QPainter* painter, const QRect& line_rect,
                                 const QFont& font, const QString& text,
                                 int width) const {
  QFont smaller = font;
  smaller.setPointSize(smaller.pointSize() - 1);
  smaller.setBold(true);

  if (width == -1) width = QFontMetrics(font).width(text + "  ");

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
  painter->translate(0.5, 0.5);
  painter->setPen(QPen(Qt::white, 1));
  painter->setBrush(gradient);
  painter->drawRoundedRect(rect, kQueueBoxCornerRadius, kQueueBoxCornerRadius);

  // Draw the text
  painter->setFont(smaller);
  painter->drawText(rect, Qt::AlignCenter, text);
  painter->translate(-0.5, -0.5);
}

int QueuedItemDelegate::queue_indicator_size(const QModelIndex& index) const {
  if (index.column() == indicator_column_) {
    const int queue_pos = index.data(Playlist::Role_QueuePosition).toInt();
    if (queue_pos != -1) {
      return kQueueBoxLength + kQueueBoxBorder * 2;
    }
  }
  return 0;
}

PlaylistDelegateBase::PlaylistDelegateBase(QObject* parent,
                                           const QString& suffix)
    : QueuedItemDelegate(parent),
      view_(qobject_cast<QTreeView*>(parent)),
      suffix_(suffix) {}

QString PlaylistDelegateBase::displayText(const QVariant& value,
                                          const QLocale&) const {
  QString text;

  switch (static_cast<QMetaType::Type>(value.type())) {
    case QMetaType::Int: {
      int v = value.toInt();
      if (v > 0) text = QString::number(v);
      break;
    }

    case QMetaType::Float:
    case QMetaType::Double: {
      double v = value.toDouble();
      if (v > 0) text = QString::number(v);
      break;
    }

    default:
      text = value.toString();
      break;
  }

  if (!text.isNull() && !suffix_.isNull()) text += " " + suffix_;
  return text;
}

QSize PlaylistDelegateBase::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
  QSize size = QueuedItemDelegate::sizeHint(option, index);
  if (size.height() < kMinHeight) size.setHeight(kMinHeight);
  return size;
}

void PlaylistDelegateBase::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
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

QStyleOptionViewItemV4 PlaylistDelegateBase::Adjusted(
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if (!view_) return option;

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

bool PlaylistDelegateBase::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) {
  // This function is copied from QAbstractItemDelegate, and changed to show
  // displayText() in the tooltip, rather than the index's naked
  // Qt::ToolTipRole text.

  Q_UNUSED(option);

  if (!event || !view) return false;

  QHelpEvent* he = static_cast<QHelpEvent*>(event);
  QString text = displayText(index.data(), QLocale::system());

  // Special case: we want newlines in the comment tooltip
  if (index.column() == Playlist::Column_Comment) {
    text = Qt::escape(index.data(Qt::ToolTipRole).toString());
    text.replace("\\r\\n", "<br />");
    text.replace("\\n", "<br />");
    text.replace("\r\n", "<br />");
    text.replace("\n", "<br />");
  }

  if (text.isEmpty() || !he) return false;

  switch (event->type()) {
    case QEvent::ToolTip: {
      QRect displayed_text;
      QSize real_text;
      bool is_elided = false;

      real_text = sizeHint(option, index);
      displayed_text = view->visualRect(index);
      is_elided = displayed_text.width() < real_text.width();
      if (is_elided) {
        QToolTip::showText(he->globalPos(), text, view);
      } else {  // in case that another text was previously displayed
        QToolTip::hideText();
      }
      return true;
    }

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

QString LengthItemDelegate::displayText(const QVariant& value,
                                        const QLocale&) const {
  bool ok = false;
  qint64 nanoseconds = value.toLongLong(&ok);

  if (ok && nanoseconds > 0) return Utilities::PrettyTimeNanosec(nanoseconds);
  return QString::null;
}

QString SizeItemDelegate::displayText(const QVariant& value,
                                      const QLocale&) const {
  bool ok = false;
  int bytes = value.toInt(&ok);

  if (ok) return Utilities::PrettySize(bytes);
  return QString();
}

QString DateItemDelegate::displayText(const QVariant& value,
                                      const QLocale& locale) const {
  bool ok = false;
  int time = value.toInt(&ok);

  if (!ok || time == -1) return QString::null;

  return QDateTime::fromTime_t(time)
      .toString(QLocale::system().dateTimeFormat(QLocale::ShortFormat));
}

QString LastPlayedItemDelegate::displayText(const QVariant& value,
                                            const QLocale& locale) const {
  bool ok = false;
  const int time = value.toInt(&ok);

  if (!ok || time == -1) return tr("Never");

  return Utilities::Ago(time, locale);
}

QString FileTypeItemDelegate::displayText(const QVariant& value,
                                          const QLocale& locale) const {
  bool ok = false;
  Song::FileType type = Song::FileType(value.toInt(&ok));

  if (!ok) return tr("Unknown");

  return Song::TextForFiletype(type);
}

QWidget* TextItemDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
  return new QLineEdit(parent);
}

RatingItemDelegate::RatingItemDelegate(QObject* parent)
    : PlaylistDelegateBase(parent) {}

void RatingItemDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
  // Draw the background
  const QStyleOptionViewItemV3* vopt =
      qstyleoption_cast<const QStyleOptionViewItemV3*>(&option);
  vopt->widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, vopt,
                                       painter, vopt->widget);

  // Don't draw anything else if the user can't set the rating of this item
  if (!index.data(Playlist::Role_CanSetRating).toBool()) return;

  const bool hover = mouse_over_index_.isValid() &&
                     (mouse_over_index_ == index ||
                      (selected_indexes_.contains(mouse_over_index_) &&
                       selected_indexes_.contains(index)));

  const double rating =
      (hover ? RatingPainter::RatingForPos(mouse_over_pos_, option.rect)
             : index.data().toDouble());

  painter_.Paint(painter, option.rect, rating);
}

QSize RatingItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  QSize size = PlaylistDelegateBase::sizeHint(option, index);
  size.setWidth(size.height() * RatingPainter::kStarCount);
  return size;
}

QString RatingItemDelegate::displayText(const QVariant& value,
                                        const QLocale&) const {
  if (value.isNull() || value.toDouble() <= 0) return QString();

  // Round to the nearest 0.5
  const double rating =
      float(int(value.toDouble() * RatingPainter::kStarCount * 2 + 0.5)) / 2;

  return QString::number(rating, 'f', 1);
}

TagCompletionModel::TagCompletionModel(LibraryBackend* backend,
                                       Playlist::Column column)
    : QStringListModel() {
  QString col = database_column(column);
  if (!col.isEmpty()) {
    setStringList(backend->GetAll(col));
  }
}

QString TagCompletionModel::database_column(Playlist::Column column) {
  switch (column) {
    case Playlist::Column_Artist:
      return "artist";
    case Playlist::Column_Album:
      return "album";
    case Playlist::Column_AlbumArtist:
      return "albumartist";
    case Playlist::Column_Composer:
      return "composer";
    case Playlist::Column_Performer:
      return "performer";
    case Playlist::Column_Grouping:
      return "grouping";
    case Playlist::Column_Genre:
      return "genre";
    default:
      qLog(Warning) << "Unknown column" << column;
      return QString();
  }
}

static TagCompletionModel* InitCompletionModel(LibraryBackend* backend,
                                               Playlist::Column column) {
  return new TagCompletionModel(backend, column);
}

TagCompleter::TagCompleter(LibraryBackend* backend, Playlist::Column column,
                           QLineEdit* editor)
    : QCompleter(editor), editor_(editor) {
  QFuture<TagCompletionModel*> future =
      QtConcurrent::run(&InitCompletionModel, backend, column);
  QFutureWatcher<TagCompletionModel*>* watcher =
      new QFutureWatcher<TagCompletionModel*>(this);
  watcher->setFuture(future);

  connect(watcher, SIGNAL(finished()), SLOT(ModelReady()));
}

void TagCompleter::ModelReady() {
  QFutureWatcher<TagCompletionModel*>* watcher =
      dynamic_cast<QFutureWatcher<TagCompletionModel*>*>(sender());
  if (!watcher) return;

  TagCompletionModel* model = watcher->result();
  setModel(model);
  setCaseSensitivity(Qt::CaseInsensitive);
  editor_->setCompleter(this);
}

QWidget* TagCompletionItemDelegate::createEditor(QWidget* parent,
                                                 const QStyleOptionViewItem&,
                                                 const QModelIndex&) const {

  QLineEdit* editor = new QLineEdit(parent);
  new TagCompleter(backend_, column_, editor);

  return editor;
}

QString NativeSeparatorsDelegate::displayText(const QVariant& value,
                                              const QLocale&) const {
  const QString string_value = value.toString();

  QUrl url;
  if (value.type() == QVariant::Url) {
    url = value.toUrl();
  } else if (string_value.contains("://")) {
    url = QUrl::fromEncoded(string_value.toAscii());
  } else {
    return QDir::toNativeSeparators(string_value);
  }

  if (url.scheme() == "file") {
    return QDir::toNativeSeparators(url.toLocalFile());
  }
  return string_value;
}

SongSourceDelegate::SongSourceDelegate(QObject* parent, Player* player)
    : PlaylistDelegateBase(parent), player_(player) {}

QString SongSourceDelegate::displayText(const QVariant& value,
                                        const QLocale&) const {
  return QString();
}

QPixmap SongSourceDelegate::LookupPixmap(const QUrl& url,
                                         const QSize& size) const {
  QPixmap pixmap;
  if (cache_.find(url.scheme(), &pixmap)) {
    return pixmap;
  }

  QIcon icon;
  const UrlHandler* handler = player_->HandlerForUrl(url);
  if (handler) {
    icon = handler->icon();
  } else {
    if (url.scheme() == "spotify") {
      icon = QIcon(":icons/22x22/spotify.png");
    } else if (url.scheme() == "file") {
      icon = IconLoader::Load("folder-sound");
    } else if (url.host() == "api.jamendo.com") {
      icon = QIcon(":/providers/jamendo.png");
    } else if (url.host() == "api.soundcloud.com") {
      icon = QIcon(":/providers/soundcloud.png");
    }
  }
  pixmap = icon.pixmap(size.height());
  cache_.insert(url.scheme(), pixmap);
  return pixmap;
}

void SongSourceDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
  // Draw the background
  PlaylistDelegateBase::paint(painter, option, index);

  QStyleOptionViewItem option_copy(option);
  initStyleOption(&option_copy, index);

  // Find the pixmap to use for this URL
  const QUrl& url = index.data().toUrl();
  QPixmap pixmap = LookupPixmap(url, option_copy.decorationSize);

  float device_pixel_ratio = 1.0f;
#ifdef Q_OS_DARWIN
  QWidget* parent_widget = reinterpret_cast<QWidget*>(parent());
  device_pixel_ratio = mac::GetDevicePixelRatio(parent_widget);
#endif

  // Draw the pixmap in the middle of the rectangle
  QRect draw_rect(QPoint(0, 0),
                  option_copy.decorationSize / device_pixel_ratio);
  draw_rect.moveCenter(option_copy.rect.center());

  painter->drawPixmap(draw_rect, pixmap);
}
