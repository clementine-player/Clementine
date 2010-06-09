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
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "widgets/trackslider.h"

#include <QDateTime>
#include <QLineEdit>
#include <QPainter>
#include <QToolTip>
#include <QWhatsThis>
#include <QHelpEvent>
#include <QHeaderView>
#include <QScrollBar>

const int PlaylistDelegateBase::kMinHeight = 19;

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

QSize PlaylistDelegateBase::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  if (size.height() < kMinHeight)
    size.setHeight(kMinHeight);
  return size;
}

void PlaylistDelegateBase::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QStyledItemDelegate::paint(painter, Adjusted(option, index), index);

  QPoint top_left(-view_->horizontalScrollBar()->value(),
                  -view_->verticalScrollBar()->value());

  if (view_->header()->logicalIndexAt(top_left) == index.column()) {
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

QString DateItemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  bool ok = false;
  int time = value.toInt(&ok);

  if (!ok || time == -1)
    return QString::null;

  return QDateTime::fromTime_t(time).toString(
      QLocale::system().dateTimeFormat(QLocale::ShortFormat));
}

QString FileTypeItemDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  bool ok = false;
  Song::FileType type = Song::FileType(value.toInt(&ok));

  if (!ok)
    return tr("Unknown");

  switch (type) {
    case Song::Type_Asf:       return tr("ASF");
    case Song::Type_Flac:      return tr("FLAC");
    case Song::Type_Mp4:       return tr("MP4");
    case Song::Type_Mpc:       return tr("MPC");
    case Song::Type_Mpeg:      return tr("MP3"); // Not technically correct
    case Song::Type_OggFlac:   return tr("Ogg FLAC");
    case Song::Type_OggSpeex:  return tr("Ogg Speex");
    case Song::Type_OggVorbis: return tr("Ogg Vorbis");
    case Song::Type_Aiff:      return tr("AIFF");
    case Song::Type_Wav:       return tr("WAV");
    case Song::Type_TrueAudio: return tr("TrueAudio");

    case Song::Type_Stream:    return tr("Stream");

    case Song::Type_Unknown:
    default:
      return tr("Unknown");
  }
}

QWidget* TextItemDelegate::createEditor(
    QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  return new QLineEdit(parent);
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
