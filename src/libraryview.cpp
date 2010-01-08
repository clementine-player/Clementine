#include "library.h"
#include "libraryview.h"
#include "libraryitem.h"

#include <QPainter>

const int LibraryView::kRowsToShow = 50;

LibraryItemDelegate::LibraryItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const {
  LibraryItem::Type type =
      static_cast<LibraryItem::Type>(index.data(Library::Role_Type).toInt());

  switch (type) {
    case LibraryItem::Type_Divider: {
      QString text(index.data().toString().toUpper());

      // Draw the background
      //QStyledItemDelegate::paint(painter, opt, QModelIndex());

      painter->save();

      // Draw the text
      QFont bold_font(opt.font);
      bold_font.setBold(true);

      QRect text_rect(opt.rect);
      text_rect.setLeft(text_rect.left() + 30);

      painter->setPen(opt.palette.color(QPalette::Text));
      painter->setFont(bold_font);
      painter->drawText(text_rect, text);

      //Draw the line under the item
      QPen line_pen(opt.palette.color(QPalette::Disabled, QPalette::Text).lighter());
      line_pen.setWidth(2);

      painter->setPen(line_pen);
      painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());

      painter->restore();
      break;
    }

    default:
      QStyledItemDelegate::paint(painter, opt, index);
      break;
  }
}

LibraryView::LibraryView(QWidget* parent)
  : QTreeView(parent),
    library_(NULL),
    total_song_count_(-1),
    nomusic_(":nomusic.png")
{
  setItemDelegate(new LibraryItemDelegate(this));
}

void LibraryView::SetLibrary(Library *library) {
  library_ = library;
}

void LibraryView::TotalSongCountUpdated(int count) {
  bool old = total_song_count_;
  total_song_count_ = count;
  if (old != total_song_count_)
    update();

  if (total_song_count_ == 0)
    setCursor(Qt::PointingHandCursor);
  else
    unsetCursor();
}

void LibraryView::reset() {
  QTreeView::reset();

  // Expand nodes in the tree until we have about 50 rows visible in the view
  int rows = model()->rowCount(rootIndex());
  RecursivelyExpand(rootIndex(), &rows);
}

void LibraryView::paintEvent(QPaintEvent* event) {
  QTreeView::paintEvent(event);
  QPainter p(viewport());

  QRect rect(viewport()->rect());
  if (total_song_count_ == 0) {
    // Draw the confused clementine
    QRect image_rect((rect.width() - nomusic_.width()) / 2, 50,
                     nomusic_.width(), nomusic_.height());
    p.drawPixmap(image_rect, nomusic_);

    // Draw the title text
    QFont bold_font;
    bold_font.setBold(true);
    p.setFont(bold_font);

    QFontMetrics metrics(bold_font);

    QRect title_rect(0, image_rect.bottom() + 20, rect.width(), metrics.height());
    p.drawText(title_rect, Qt::AlignHCenter, "Your library is empty!");

    // Draw the other text
    p.setFont(QFont());

    QRect text_rect(0, title_rect.bottom() + 5, rect.width(), metrics.height());
    p.drawText(text_rect, Qt::AlignHCenter, "Click here to add some music");
  }
}

void LibraryView::mouseReleaseEvent(QMouseEvent* e) {
  QTreeView::mouseReleaseEvent(e);

  if (total_song_count_ == 0) {
    emit ShowConfigDialog();
  }
}

bool LibraryView::RecursivelyExpand(const QModelIndex& index, int* count) {
  int children = model()->rowCount(index);
  if (*count + children > kRowsToShow)
    return false;

  expand(index);
  *count += children;

  for (int i=0 ; i<children ; ++i) {
    if (!RecursivelyExpand(model()->index(i, 0, index), count))
      return false;
  }

  return true;
}
