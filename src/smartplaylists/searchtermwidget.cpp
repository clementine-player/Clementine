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

#include "searchterm.h"
#include "searchtermwidget.h"
#include "ui_searchtermwidget.h"
#include "core/utilities.h"
#include "playlist/playlist.h"
#include "playlist/playlistdelegates.h"
#include "ui/iconloader.h"

#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QtDebug>
#include <QKeyEvent>

// Exported by QtGui
void qt_blurImage(QPainter* p, QImage& blurImage, qreal radius, bool quality,
                  bool alphaOnly, int transposed = 0);

namespace smart_playlists {

class SearchTermWidget::Overlay : public QWidget {
 public:
  Overlay(SearchTermWidget* parent);
  void Grab();
  void SetOpacity(float opacity);
  float opacity() const { return opacity_; }

  static const int kSpacing;
  static const int kIconSize;

 protected:
  void paintEvent(QPaintEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void keyReleaseEvent(QKeyEvent* e);

 private:
  SearchTermWidget* parent_;

  float opacity_;
  QString text_;
  QPixmap pixmap_;
  QPixmap icon_;
};

const int SearchTermWidget::Overlay::kSpacing = 6;
const int SearchTermWidget::Overlay::kIconSize = 22;

SearchTermWidget::SearchTermWidget(LibraryBackend* library, QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_SmartPlaylistSearchTermWidget),
      library_(library),
      overlay_(nullptr),
      animation_(new QPropertyAnimation(this, "overlay_opacity", this)),
      active_(true),
      initialized_(false),
      current_field_type_(SearchTerm::Type_Invalid) {
  ui_->setupUi(this);
  connect(ui_->field, SIGNAL(currentIndexChanged(int)),
          SLOT(FieldChanged(int)));
  connect(ui_->op, SIGNAL(currentIndexChanged(int)), SLOT(OpChanged(int)));
  connect(ui_->remove, SIGNAL(clicked()), SIGNAL(RemoveClicked()));

  connect(ui_->value_date, SIGNAL(dateChanged(QDate)), SIGNAL(Changed()));
  connect(ui_->value_number, SIGNAL(valueChanged(int)), SIGNAL(Changed()));
  connect(ui_->value_rating, SIGNAL(RatingChanged(float)), SIGNAL(Changed()));
  connect(ui_->value_text, SIGNAL(textChanged(QString)), SIGNAL(Changed()));
  connect(ui_->value_time, SIGNAL(timeChanged(QTime)), SIGNAL(Changed()));
  connect(ui_->value_date_numeric, SIGNAL(valueChanged(int)),
          SIGNAL(Changed()));
  connect(ui_->value_date_numeric1, SIGNAL(valueChanged(int)),
          SLOT(RelativeValueChanged()));
  connect(ui_->value_date_numeric2, SIGNAL(valueChanged(int)),
          SLOT(RelativeValueChanged()));
  connect(ui_->date_type, SIGNAL(currentIndexChanged(int)), SIGNAL(Changed()));
  connect(ui_->date_type_relative, SIGNAL(currentIndexChanged(int)),
          SIGNAL(Changed()));

  ui_->value_date->setDate(QDate::currentDate());

  // Populate the combo boxes
  for (int i = 0; i < SearchTerm::FieldCount; ++i) {
    ui_->field->addItem(SearchTerm::FieldName(SearchTerm::Field(i)));
    ui_->field->setItemData(i, i);
  }
  ui_->field->model()->sort(0);

  // Populate the date type combo box
  for (int i = 0; i < 5; ++i) {
    ui_->date_type->addItem(
        SearchTerm::DateName(SearchTerm::DateType(i), false));
    ui_->date_type->setItemData(i, i);

    ui_->date_type_relative->addItem(
        SearchTerm::DateName(SearchTerm::DateType(i), false));
    ui_->date_type_relative->setItemData(i, i);
  }

  // Icons on the buttons
  ui_->remove->setIcon(IconLoader::Load("list-remove", IconLoader::Base));

  // Set stylesheet
  QFile stylesheet_file(":/smartplaylistsearchterm.css");
  stylesheet_file.open(QIODevice::ReadOnly);
  QString stylesheet = QString::fromAscii(stylesheet_file.readAll());
  const QColor base(222, 97, 97, 128);
  stylesheet.replace("%light2", Utilities::ColorToRgba(base.lighter(140)));
  stylesheet.replace("%light", Utilities::ColorToRgba(base.lighter(120)));
  stylesheet.replace("%dark", Utilities::ColorToRgba(base.darker(120)));
  stylesheet.replace("%base", Utilities::ColorToRgba(base));
  setStyleSheet(stylesheet);
}

SearchTermWidget::~SearchTermWidget() { delete ui_; }

void SearchTermWidget::FieldChanged(int index) {
  SearchTerm::Field field =
      SearchTerm::Field(ui_->field->itemData(index).toInt());
  SearchTerm::Type type = SearchTerm::TypeOf(field);

  // Populate the operator combo box
  if (type != current_field_type_) {
    ui_->op->clear();
    for (SearchTerm::Operator op : SearchTerm::OperatorsForType(type)) {
      const int i = ui_->op->count();
      ui_->op->addItem(SearchTerm::OperatorText(type, op));
      ui_->op->setItemData(i, op);
    }
    current_field_type_ = type;
  }

  // Show the correct value editor
  QWidget* page = nullptr;
  switch (type) {
    case SearchTerm::Type_Time:
      page = ui_->page_time;
      break;
    case SearchTerm::Type_Number:
      page = ui_->page_number;
      break;
    case SearchTerm::Type_Date:
      page = ui_->page_date;
      break;
    case SearchTerm::Type_Rating:
      page = ui_->page_rating;
      break;
    case SearchTerm::Type_Text:
      if (ui_->op->currentIndex() == 4 || ui_->op->currentIndex() == 5) {
        page = ui_->page_empty;
      } else {
        page = ui_->page_text;
      }
      break;
    case SearchTerm::Type_Invalid:
      page = nullptr;
      break;
  }
  ui_->value_stack->setCurrentWidget(page);

  // Maybe set a tag completer
  switch (field) {
    case SearchTerm::Field_Artist:
      new TagCompleter(library_, Playlist::Column_Artist, ui_->value_text);
      break;

    case SearchTerm::Field_Album:
      new TagCompleter(library_, Playlist::Column_Album, ui_->value_text);
      break;

    default:
      ui_->value_text->setCompleter(nullptr);
  }

  emit Changed();
}

void SearchTermWidget::OpChanged(int index) {
  if ((ui_->value_stack->currentWidget() == ui_->page_text) ||
      (ui_->value_stack->currentWidget() == ui_->page_empty)) {
    QWidget* page = nullptr;
    // This assumes the operators always appear in the same order.
    // Needs a better way for checking which is the current operator.
    if (index == 4 || index == 5) {
      page = ui_->page_empty;
    } else {
      page = ui_->page_text;
    }
    ui_->value_stack->setCurrentWidget(page);
  }

  // We need to change the page only in the following case
  else if ((ui_->value_stack->currentWidget() == ui_->page_date) ||
      (ui_->value_stack->currentWidget() == ui_->page_date_numeric) ||
      (ui_->value_stack->currentWidget() == ui_->page_date_relative)) {
    QWidget* page = nullptr;
    if (index == 4 || index == 5) {
      page = ui_->page_date_numeric;
    } else if (index == 6) {
      page = ui_->page_date_relative;
    } else {
      page = ui_->page_date;
    }
    ui_->value_stack->setCurrentWidget(page);
  }

  emit Changed();
}

void SearchTermWidget::SetActive(bool active) {
  active_ = active;
  delete overlay_;
  overlay_ = nullptr;

  ui_->container->setEnabled(active);

  if (!active) {
    overlay_ = new Overlay(this);
  }
}

void SearchTermWidget::enterEvent(QEvent*) {
  if (!overlay_ || !isEnabled()) return;

  animation_->stop();
  animation_->setEndValue(1.0);
  animation_->setDuration(80);
  animation_->start();
}

void SearchTermWidget::leaveEvent(QEvent*) {
  if (!overlay_) return;

  animation_->stop();
  animation_->setEndValue(0.0);
  animation_->setDuration(160);
  animation_->start();
}

void SearchTermWidget::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);
  if (overlay_ && overlay_->isVisible()) {
    QTimer::singleShot(0, this, SLOT(Grab()));
  }
}

void SearchTermWidget::showEvent(QShowEvent* e) {
  QWidget::showEvent(e);
  if (overlay_) {
    QTimer::singleShot(0, this, SLOT(Grab()));
  }
}

void SearchTermWidget::Grab() { overlay_->Grab(); }

void SearchTermWidget::set_overlay_opacity(float opacity) {
  if (overlay_) overlay_->SetOpacity(opacity);
}

float SearchTermWidget::overlay_opacity() const {
  return overlay_ ? overlay_->opacity() : 0.0;
}

void SearchTermWidget::SetTerm(const SearchTerm& term) {
  ui_->field->setCurrentIndex(ui_->field->findData(term.field_));
  ui_->op->setCurrentIndex(ui_->op->findData(term.operator_));

  // The value depends on the data type
  switch (SearchTerm::TypeOf(term.field_)) {
    case SearchTerm::Type_Text:
      if (ui_->value_stack->currentWidget() == ui_->page_empty) {
        ui_->value_text->setText("");
      } else {
        ui_->value_text->setText(term.value_.toString());
      }
      break;

    case SearchTerm::Type_Number:
      ui_->value_number->setValue(term.value_.toInt());
      break;

    case SearchTerm::Type_Date:
      if (ui_->value_stack->currentWidget() == ui_->page_date_numeric) {
        ui_->value_date_numeric->setValue(term.value_.toInt());
        ui_->date_type->setCurrentIndex(term.date_);
      } else if (ui_->value_stack->currentWidget() == ui_->page_date_relative) {
        ui_->value_date_numeric1->setValue(term.value_.toInt());
        ui_->value_date_numeric2->setValue(term.second_value_.toInt());
        ui_->date_type_relative->setCurrentIndex(term.date_);
      } else if (ui_->value_stack->currentWidget() == ui_->page_date) {
        ui_->value_date->setDateTime(
            QDateTime::fromTime_t(term.value_.toInt()));
      }
      break;

    case SearchTerm::Type_Time:
      ui_->value_time->setTime(QTime(0, 0).addSecs(term.value_.toInt()));
      break;

    case SearchTerm::Type_Rating:
      ui_->value_rating->set_rating(term.value_.toFloat());
      break;

    case SearchTerm::Type_Invalid:
      break;
  }
}

SearchTerm SearchTermWidget::Term() const {
  const int field = ui_->field->itemData(ui_->field->currentIndex()).toInt();
  const int op = ui_->op->itemData(ui_->op->currentIndex()).toInt();

  SearchTerm ret;
  ret.field_ = SearchTerm::Field(field);
  ret.operator_ = SearchTerm::Operator(op);

  // The value depends on the data type
  const QWidget* value_page = ui_->value_stack->currentWidget();
  if (value_page == ui_->page_text) {
    ret.value_ = ui_->value_text->text();
  } else if (value_page == ui_->page_empty) {
    ret.value_ = "";
  } else if (value_page == ui_->page_number) {
    ret.value_ = ui_->value_number->value();
  } else if (value_page == ui_->page_date) {
    ret.value_ = ui_->value_date->dateTime().toTime_t();
  } else if (value_page == ui_->page_time) {
    ret.value_ = QTime(0, 0).secsTo(ui_->value_time->time());
  } else if (value_page == ui_->page_rating) {
    ret.value_ = ui_->value_rating->rating();
  } else if (value_page == ui_->page_date_numeric) {
    ret.date_ = SearchTerm::DateType(ui_->date_type->currentIndex());
    ret.value_ = ui_->value_date_numeric->value();
  } else if (value_page == ui_->page_date_relative) {
    ret.date_ = SearchTerm::DateType(ui_->date_type_relative->currentIndex());
    ret.value_ = ui_->value_date_numeric1->value();
    ret.second_value_ = ui_->value_date_numeric2->value();
  }

  return ret;
}

void SearchTermWidget::RelativeValueChanged() {
  // Don't check for validity when creating the widget
  if (!initialized_) {
    initialized_ = true;
    return;
  }
  // Explain the user why he can't proceed
  if (ui_->value_date_numeric1->value() >= ui_->value_date_numeric2->value()) {
    QMessageBox::warning(
        this, tr("Clementine"),
        tr("The second value must be greater than the first one!"));
  }
  // Emit the signal in any case, so the Next button will be disabled
  emit Changed();
}

SearchTermWidget::Overlay::Overlay(SearchTermWidget* parent)
    : QWidget(parent),
      parent_(parent),
      opacity_(0.0),
      text_(tr("Add search term")),
      icon_(IconLoader::Load("list-add", IconLoader::Base).pixmap(kIconSize)) {
  raise();
  setFocusPolicy(Qt::TabFocus);
}

void SearchTermWidget::Overlay::SetOpacity(float opacity) {
  opacity_ = opacity;
  update();
}

void SearchTermWidget::Overlay::Grab() {
  hide();

  // Take a "screenshot" of the window
  QPixmap pixmap = QPixmap::grabWidget(parent_);
  QImage image = pixmap.toImage();

  // Blur it
  QImage blurred(image.size(), QImage::Format_ARGB32_Premultiplied);
  blurred.fill(Qt::transparent);

  QPainter blur_painter(&blurred);
  qt_blurImage(&blur_painter, image, 10.0, true, false);
  blur_painter.end();

  pixmap_ = QPixmap::fromImage(blurred);

  resize(parent_->size());
  show();
  update();
}

void SearchTermWidget::Overlay::paintEvent(QPaintEvent*) {
  QPainter p(this);

  // Background
  p.fillRect(rect(), palette().window());

  // Blurred parent widget
  p.setOpacity(0.25 + opacity_ * 0.25);
  p.drawPixmap(0, 0, pixmap_);

  // Draw a frame
  p.setOpacity(1.0);
  p.setPen(palette().color(QPalette::Mid));
  p.setRenderHint(QPainter::Antialiasing);
  p.drawRoundedRect(rect(), 5, 5);

  // Geometry
  const QSize contents_size(kIconSize + kSpacing + fontMetrics().width(text_),
                            qMax(kIconSize, fontMetrics().height()));
  const QRect contents(QPoint((width() - contents_size.width()) / 2,
                              (height() - contents_size.height()) / 2),
                       contents_size);
  const QRect icon(contents.topLeft(), QSize(kIconSize, kIconSize));
  const QRect text(icon.right() + kSpacing, icon.top(),
                   contents.width() - kSpacing - kIconSize, contents.height());

  // Icon and text
  p.setPen(palette().color(QPalette::Text));
  p.drawPixmap(icon, icon_);
  p.drawText(text, Qt::TextDontClip | Qt::AlignVCenter, text_);
}

void SearchTermWidget::Overlay::mouseReleaseEvent(QMouseEvent*) {
  emit parent_->Clicked();
}

void SearchTermWidget::Overlay::keyReleaseEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Space) emit parent_->Clicked();
}

}  // namespace
