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

#include "config.h"
#include "globalsearch.h"
#include "globalsearchitemdelegate.h"
#include "globalsearchsortmodel.h"
#include "globalsearchwidget.h"
#include "librarysearchprovider.h"
#include "ui_globalsearchwidget.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "playlist/songmimedata.h"
#include "widgets/stylehelper.h"

#ifdef HAVE_SPOTIFY
# include "spotifysearchprovider.h"
#endif

#include <QListView>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>


const int GlobalSearchWidget::kMinVisibleItems = 3;
const int GlobalSearchWidget::kMaxVisibleItems = 12;


GlobalSearchWidget::GlobalSearchWidget(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_GlobalSearchWidget),
    engine_(new GlobalSearch(this)),
    last_id_(0),
    clear_model_on_next_result_(false),
    model_(new QStandardItemModel(this)),
    proxy_(new GlobalSearchSortModel(this)),
    view_(new QListView),
    eat_focus_out_(false),
    background_(":allthethings.png")
{
  ui_->setupUi(this);

  proxy_->setSourceModel(model_);
  proxy_->setDynamicSortFilter(true);
  proxy_->sort(0);

  view_->setWindowFlags(Qt::Popup);
  view_->setFocusPolicy(Qt::NoFocus);
  view_->setFocusProxy(ui_->search);
  view_->installEventFilter(this);

  view_->setModel(proxy_);
  view_->setItemDelegate(new GlobalSearchItemDelegate(this));
  view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view_->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(ui_->search, SIGNAL(textEdited(QString)), SLOT(TextEdited(QString)));
  connect(engine_, SIGNAL(ResultsAvailable(int,SearchProvider::ResultList)),
          SLOT(AddResults(int,SearchProvider::ResultList)));
  connect(engine_, SIGNAL(SearchFinished(int)), SLOT(SearchFinished(int)));
  connect(engine_, SIGNAL(ArtLoaded(int,QPixmap)), SLOT(ArtLoaded(int,QPixmap)));
  connect(engine_, SIGNAL(TracksLoaded(int,MimeData*)), SLOT(TracksLoaded(int,MimeData*)));
  connect(view_, SIGNAL(doubleClicked(QModelIndex)), SLOT(AddCurrent()));
}

GlobalSearchWidget::~GlobalSearchWidget() {
  delete ui_;
}

void GlobalSearchWidget::Init(LibraryBackendInterface* library) {
  // Add providers
  engine_->AddProvider(new LibrarySearchProvider(
      library, tr("Library"), IconLoader::Load("folder-sound"), engine_));

#ifdef HAVE_SPOTIFY
  engine_->AddProvider(new SpotifySearchProvider(engine_));
#endif

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
  const QString trimmed_text = text.trimmed();

  if (trimmed_text.length() < 3) {
    Reset();
    RepositionPopup();
    return;
  }

  clear_model_on_next_result_ = true;
  engine_->CancelSearch(last_id_);
  last_id_ = engine_->SearchAsync(trimmed_text);
}

void GlobalSearchWidget::Reset() {
  model_->clear();
  art_requests_.clear();
}

void GlobalSearchWidget::SearchFinished(int id) {
  if (id != last_id_)
    return;

  if (clear_model_on_next_result_) {
    Reset();
    clear_model_on_next_result_ = true;
  }

  RepositionPopup();
}

void GlobalSearchWidget::AddResults(int id, const SearchProvider::ResultList& results) {
  if (id != last_id_)
    return;

  if (clear_model_on_next_result_) {
    Reset();
    clear_model_on_next_result_ = false;
  }

  foreach (const SearchProvider::Result& result, results) {
    QStandardItem* item = new QStandardItem;
    item->setData(QVariant::fromValue(result), Role_Result);

    QPixmap pixmap;
    if (engine_->FindCachedPixmap(result, &pixmap)) {
      item->setData(pixmap, Qt::DecorationRole);
    }

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
        view_->setCurrentIndex(proxy_->index(proxy_->rowCount() - 1, 0));
        return true;
      } else if (cur_index.row() == 0) {
        return true;
      }
      return false;

    case Qt::Key_Down:
      if (!cur_index.isValid()) {
        view_->setCurrentIndex(proxy_->index(0, 0));
        return true;
      } else if (cur_index.row() == proxy_->rowCount() - 1) {
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
      AddCurrent();
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

void GlobalSearchWidget::LazyLoadArt(const QModelIndex& proxy_index) {
  if (!proxy_index.isValid() || proxy_index.data(Role_LazyLoadingArt).isValid()) {
    return;
  }

  const QModelIndex source_index = proxy_->mapToSource(proxy_index);

  model_->itemFromIndex(source_index)->setData(true, Role_LazyLoadingArt);

  const SearchProvider::Result result =
      source_index.data(Role_Result).value<SearchProvider::Result>();

  int id = engine_->LoadArtAsync(result);
  art_requests_[id] = source_index;
}

void GlobalSearchWidget::ArtLoaded(int id, const QPixmap& pixmap) {
  if (!art_requests_.contains(id))
    return;
  QModelIndex index = art_requests_.take(id);

  model_->itemFromIndex(index)->setData(pixmap, Qt::DecorationRole);
}

void GlobalSearchWidget::AddCurrent() {
  QModelIndex index = view_->currentIndex();
  if (!index.isValid())
    index = proxy_->index(0, 0);

  if (!index.isValid())
    return;

  engine_->LoadTracksAsync(index.data(Role_Result).value<SearchProvider::Result>());
}

void GlobalSearchWidget::TracksLoaded(int id, MimeData* mime_data) {
  Q_UNUSED(id);

  if (!mime_data)
    return;

  mime_data->from_doubleclick_ = true;
  emit AddToPlaylist(mime_data);
}

