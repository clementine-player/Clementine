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

#include "globalsearch.h"
#include "globalsearchwidget.h"
#include "librarysearchprovider.h"
#include "ui_globalsearchwidget.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "widgets/stylehelper.h"

#include <QListView>
#include <QPainter>
#include <QStandardItemModel>

const int GlobalSearchItemDelegate::kHeight = SearchProvider::kArtHeight;
const int GlobalSearchItemDelegate::kMargin = 1;
const int GlobalSearchItemDelegate::kArtMargin = 6;
const int GlobalSearchItemDelegate::kWordPadding = 6;
const int GlobalSearchWidget::kMinVisibleItems = 3;
const int GlobalSearchWidget::kMaxVisibleItems = 12;


GlobalSearchItemDelegate::GlobalSearchItemDelegate(GlobalSearchWidget* widget)
  : QStyledItemDelegate(widget),
    widget_(widget)
{
  no_cover_ = ScaleAndPad(QImage(":nocover.png"));
}

QPixmap GlobalSearchItemDelegate::ScaleAndPad(const QImage& image) {
  if (image.isNull())
    return QPixmap();

  if (image.size() == QSize(kHeight, kHeight))
    return QPixmap::fromImage(image);

  // Scale the image down
  QImage copy;
  copy = image.scaled(QSize(kHeight, kHeight),
                      Qt::KeepAspectRatio, Qt::SmoothTransformation);

  // Pad the image to kHeight x kHeight
  QImage padded_image(kHeight, kHeight, QImage::Format_ARGB32);
  padded_image.fill(0);

  QPainter p(&padded_image);
  p.drawImage((kHeight - copy.width()) / 2, (kHeight - copy.height()) / 2,
              copy);
  p.end();

  return QPixmap::fromImage(padded_image);
}

QSize GlobalSearchItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const {
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  size.setHeight(kHeight + kMargin);
  return size;
}

void GlobalSearchItemDelegate::DrawAndShrink(QPainter* p, QRect* rect,
                                             const QString& text) const {
  QRect br;
  p->drawText(*rect, Qt::TextSingleLine | Qt::AlignVCenter, text, &br);
  rect->setLeft(br.right() + kWordPadding);
}

void GlobalSearchItemDelegate::paint(QPainter* p,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
  const SearchProvider::Result result =
      index.data(GlobalSearchWidget::Role_Result).value<SearchProvider::Result>();
  const Song& m = result.metadata_;

  widget_->LazyLoadArt(index);

  QFont bold_font = option.font;
  bold_font.setBold(true);

  QColor pen = option.palette.color(QPalette::Text);
  QColor light_pen = pen;
  pen.setAlpha(200);
  light_pen.setAlpha(128);

  // Draw the background
  const QStyleOptionViewItemV3* vopt = qstyleoption_cast<const QStyleOptionViewItemV3*>(&option);
  const QWidget* widget = vopt->widget;
  QStyle* style = widget->style() ? widget->style() : QApplication::style();
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, widget);

  // Draw the album art.  This will already be the correct size.
  const QRect rect = option.rect;
  const QRect art_rect(rect.left() + kMargin, rect.top(), kHeight, kHeight);

  QPixmap art = index.data(Qt::DecorationRole).value<QPixmap>();
  if (art.isNull())
    art = no_cover_;

  p->drawPixmap(art_rect, art);

  // Position text
  QRect text_rect(art_rect.right() + kArtMargin, art_rect.top(),
                  rect.right() - art_rect.right() - kArtMargin, kHeight);
  QRect text_rect_1(text_rect.adjusted(0, 0, 0, -kHeight/2));
  QRect text_rect_2(text_rect.adjusted(0, kHeight/2, 0, 0));

  // The text we draw depends on the type of result.
  switch (result.type_) {
  case SearchProvider::Result::Type_Track: {
    // Line 1 is Title
    p->setFont(bold_font);

    // Title
    p->setPen(pen);
    DrawAndShrink(p, &text_rect_1, m.title());

    // Line 2 is Artist - Album
    p->setFont(option.font);

    // Artist
    p->setPen(pen);
    if (!m.artist().isEmpty()) {
      DrawAndShrink(p, &text_rect_2, m.artist());
    } else if (!m.albumartist().isEmpty()) {
      DrawAndShrink(p, &text_rect_2, m.albumartist());
    }

    if (!m.album().isEmpty()) {
      // Dash
      p->setPen(light_pen);
      DrawAndShrink(p, &text_rect_2, " - ");

      // Album
      p->setPen(pen);
      DrawAndShrink(p, &text_rect_2, m.album());
    }

    break;
  }

  case SearchProvider::Result::Type_Album: {
    // Line 1 is Artist - Album
    p->setFont(bold_font);

    // Artist
    p->setPen(pen);
    if (!m.albumartist().isEmpty())
      DrawAndShrink(p, &text_rect_1, m.albumartist());
    else if (m.is_compilation())
      DrawAndShrink(p, &text_rect_1, tr("Various Artists"));
    else if (!m.artist().isEmpty())
      DrawAndShrink(p, &text_rect_1, m.artist());
    else
      DrawAndShrink(p, &text_rect_1, tr("Unknown"));

    // Dash
    p->setPen(light_pen);
    DrawAndShrink(p, &text_rect_1, " - ");

    // Album
    p->setPen(pen);
    if (m.album().isEmpty())
      DrawAndShrink(p, &text_rect_1, tr("Unknown"));
    else
      DrawAndShrink(p, &text_rect_1, m.album());

    // Line 2 is <n> tracks
    p->setFont(option.font);

    p->setPen(pen);
    DrawAndShrink(p, &text_rect_2, QString::number(result.album_size_));

    p->setPen(light_pen);
    DrawAndShrink(p, &text_rect_2, tr(result.album_size_ == 1 ? "track" : "tracks"));
    break;
  }

  default:
    break;
  }
}


GlobalSearchWidget::GlobalSearchWidget(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_GlobalSearchWidget),
    engine_(new GlobalSearch(this)),
    last_id_(0),
    clear_model_on_next_result_(false),
    model_(new QStandardItemModel(this)),
    view_(new QListView),
    eat_focus_out_(false),
    background_(":allthethings.png")
{
  ui_->setupUi(this);

  view_->setWindowFlags(Qt::Popup);
  view_->setFocusPolicy(Qt::NoFocus);
  view_->setFocusProxy(ui_->search);
  view_->installEventFilter(this);

  view_->setModel(model_);
  view_->setItemDelegate(new GlobalSearchItemDelegate(this));
  view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view_->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(ui_->search, SIGNAL(textEdited(QString)), SLOT(TextEdited(QString)));
  connect(engine_, SIGNAL(ResultsAvailable(int,SearchProvider::ResultList)),
          SLOT(AddResults(int,SearchProvider::ResultList)));
  connect(engine_, SIGNAL(ArtLoaded(int,QImage)), SLOT(ArtLoaded(int,QImage)));
}

GlobalSearchWidget::~GlobalSearchWidget() {
  delete ui_;
}

void GlobalSearchWidget::Init(LibraryBackendInterface* library) {
  engine_->AddProvider(new LibrarySearchProvider(
      library, tr("Library"), IconLoader::Load("folder-sound"), engine_));

  // The style helper's base color doesn't get initialised until after the
  // constructor.
  QPalette view_palette = view_->palette();
  view_palette.setColor(QPalette::Text, Qt::white);
  view_palette.setColor(QPalette::Base, Utils::StyleHelper::shadowColor().darker(109));

  QFont view_font = view_->font();
  view_font.setPointSizeF(Utils::StyleHelper::sidebarFontSize());

  view_->setFont(view_font);
  view_->setPalette(view_palette);
}

void GlobalSearchWidget::resizeEvent(QResizeEvent* e) {
  background_scaled_ = background_.scaled(size(), Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);

  QWidget::resizeEvent(e);
}

void GlobalSearchWidget::paintEvent(QPaintEvent* e) {
  QPainter p(this);

  QRect total_rect = rect().adjusted(0, 0, 1, 0);
  total_rect = style()->visualRect(layoutDirection(), geometry(), total_rect);
  Utils::StyleHelper::verticalGradient(&p, total_rect, total_rect);

  QRect background_rect = background_scaled_.rect();
  background_rect.moveLeft(total_rect.right() - background_rect.width());
  background_rect.moveTop(total_rect.top());

  p.setOpacity(0.5);
  p.drawPixmap(background_rect, background_scaled_);
  p.setOpacity(1.0);

  p.setPen(Utils::StyleHelper::borderColor());
  p.drawLine(total_rect.topRight(), total_rect.bottomRight());

  QColor light = Utils::StyleHelper::sidebarHighlight();
  p.setPen(light);
  p.drawLine(total_rect.bottomLeft(), total_rect.bottomRight());
}

void GlobalSearchWidget::TextEdited(const QString& text) {
  clear_model_on_next_result_ = true;
  last_id_ = engine_->SearchAsync(text);
}

void GlobalSearchWidget::AddResults(int id, const SearchProvider::ResultList& results) {
  if (id != last_id_)
    return;

  if (clear_model_on_next_result_) {
    model_->clear();
    art_requests_.clear();
    clear_model_on_next_result_ = false;
  }

  foreach (const SearchProvider::Result& result, results) {
    QStandardItem* item = new QStandardItem;
    item->setData(QVariant::fromValue(result), Role_Result);

    model_->appendRow(item);
  }

  RepositionPopup();
}

void GlobalSearchWidget::RepositionPopup() {
  if (model_->rowCount() == 0) {
    view_->hide();
    return;
  }

  int h = view_->sizeHintForRow(0) * float(0.5 +
      qBound(kMinVisibleItems, model_->rowCount(), kMaxVisibleItems));
  int w = ui_->search->width();

  QPoint pos = ui_->search->mapToGlobal(ui_->search->rect().bottomLeft());

  view_->setGeometry(QRect(pos, QSize(w, h)));

  if (!view_->isVisible()) {
    view_->show();
    ui_->search->setFocus();
  }
}

bool GlobalSearchWidget::eventFilter(QObject* o, QEvent* e) {
  // Most of this is borrowed from QCompleter::eventFilter

  if (eat_focus_out_ && o == ui_->search && e->type() == QEvent::FocusOut) {
    if (view_->isVisible())
      return true;
  }

  if (o != view_)
    return QWidget::eventFilter(o, e);

  switch (e->type()) {
  case QEvent::KeyPress: {
    QKeyEvent* ke = static_cast<QKeyEvent*>(e);

    QModelIndex cur_index = view_->currentIndex();
    const int key = ke->key();

    // Handle popup navigation keys. These are hardcoded because up/down might make the
    // widget do something else (lineedit cursor moves to home/end on mac, for instance)
    switch (key) {
    case Qt::Key_End:
    case Qt::Key_Home:
      if (ke->modifiers() & Qt::ControlModifier)
        return false;
      break;

    case Qt::Key_Up:
      if (!cur_index.isValid()) {
        view_->setCurrentIndex(model_->index(model_->rowCount() - 1, 0));
        return true;
      } else if (cur_index.row() == 0) {
        return true;
      }
      return false;

    case Qt::Key_Down:
      if (!cur_index.isValid()) {
        view_->setCurrentIndex(model_->index(0, 0));
        return true;
      } else if (cur_index.row() == model_->rowCount() - 1) {
        return true;
      }
      return false;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
      return false;
    }

    // Send the event to the widget. If the widget accepted the event, do nothing
    // If the widget did not accept the event, provide a default implementation
    eat_focus_out_ = false;
    (static_cast<QObject *>(ui_->search))->event(ke);
    eat_focus_out_ = true;

    if (e->isAccepted() || !view_->isVisible()) {
      // widget lost focus, hide the popup
      if (!ui_->search->hasFocus())
        view_->hide();
      if (e->isAccepted())
        return true;
    }

    // default implementation for keys not handled by the widget when popup is open
    switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Tab:
      view_->hide();
      // TODO: complete
      break;

    case Qt::Key_F4:
      if (ke->modifiers() & Qt::AltModifier)
        view_->hide();
      break;

    case Qt::Key_Backtab:
    case Qt::Key_Escape:
      view_->hide();
      break;

    default:
      break;
    }

    return true;
  }

  case QEvent::MouseButtonPress:
    if (!view_->underMouse()) {
      view_->hide();
      return true;
    }
    return false;

  case QEvent::InputMethod:
  case QEvent::ShortcutOverride:
    QApplication::sendEvent(ui_->search, e);
    break;

  default:
    return false;
  }

  return false;
}

void GlobalSearchWidget::LazyLoadArt(const QModelIndex& index) {
  if (!index.isValid() || index.data(Role_LazyLoadingArt).isValid()) {
    return;
  }

  model_->itemFromIndex(index)->setData(true, Role_LazyLoadingArt);

  const SearchProvider::Result result =
      index.data(Role_Result).value<SearchProvider::Result>();

  int id = engine_->LoadArtAsync(result);
  art_requests_[id] = index;
}

void GlobalSearchWidget::ArtLoaded(int id, const QImage& image) {
  if (!art_requests_.contains(id))
    return;
  QModelIndex index = art_requests_.take(id);

  model_->itemFromIndex(index)->setData(
        GlobalSearchItemDelegate::ScaleAndPad(image), Qt::DecorationRole);
}

