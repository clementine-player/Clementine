/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "moodbarproxystyle.h"
#include "core/application.h"
#include "core/logging.h"

#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <QSlider>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QTimeLine>

const int MoodbarProxyStyle::kMarginSize = 3;
const int MoodbarProxyStyle::kBorderSize = 1;
const int MoodbarProxyStyle::kArrowWidth = 17;
const int MoodbarProxyStyle::kArrowHeight = 13;

MoodbarProxyStyle::MoodbarProxyStyle(Application* app, QSlider* slider)
    : QProxyStyle(slider->style()),
      app_(app),
      slider_(slider),
      enabled_(true),
      moodbar_style_(MoodbarRenderer::Style_Normal),
      state_(MoodbarOff),
      fade_timeline_(new QTimeLine(1000, this)),
      moodbar_colors_dirty_(true),
      moodbar_pixmap_dirty_(true),
      context_menu_(nullptr),
      show_moodbar_action_(nullptr),
      style_action_group_(nullptr) {
  slider->setStyle(this);
  slider->installEventFilter(this);

  connect(fade_timeline_, SIGNAL(valueChanged(qreal)),
          SLOT(FaderValueChanged(qreal)));

  connect(app, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
  ReloadSettings();
}

void MoodbarProxyStyle::ReloadSettings() {
  QSettings s;
  s.beginGroup("Moodbar");

  // Get the enabled/disabled setting, and start the timelines if there's a
  // change.
  enabled_ = s.value("show", true).toBool();
  NextState();

  // Get the style, and redraw if there's a change.
  MoodbarRenderer::MoodbarStyle new_style =
      static_cast<MoodbarRenderer::MoodbarStyle>(
          s.value("style", MoodbarRenderer::Style_Normal).toInt());

  if (new_style != moodbar_style_) {
    moodbar_style_ = new_style;
    moodbar_colors_dirty_ = true;
    slider_->update();
  }
}

void MoodbarProxyStyle::SetMoodbarData(const QByteArray& data) {
  data_ = data;
  moodbar_colors_dirty_ = true;  // Redraw next time
  NextState();
}

void MoodbarProxyStyle::SetMoodbarEnabled(bool enabled) {
  enabled_ = enabled;

  // Save the enabled setting.
  QSettings s;
  s.beginGroup("Moodbar");
  s.setValue("show", enabled);

  app_->ReloadSettings();
}

void MoodbarProxyStyle::NextState() {
  const bool visible = enabled_ && !data_.isEmpty();

  // While the regular slider should stay at the standard size (Fixed),
  // moodbars should use all available space (MinimumExpanding).
  slider_->setSizePolicy(
      QSizePolicy::Expanding,
      visible ? QSizePolicy::MinimumExpanding : QSizePolicy::Fixed);
  slider_->updateGeometry();

  if (show_moodbar_action_) {
    show_moodbar_action_->setChecked(enabled_);
  }

  if ((visible && (state_ == MoodbarOn || state_ == FadingToOn)) ||
      (!visible && (state_ == MoodbarOff || state_ == FadingToOff))) {
    return;
  }

  const QTimeLine::Direction direction =
      visible ? QTimeLine::Forward : QTimeLine::Backward;

  if (state_ == MoodbarOn || state_ == MoodbarOff) {
    // Start the fade from the beginning.
    fade_timeline_->setDirection(direction);
    fade_timeline_->start();

    fade_source_ = QPixmap();
    fade_target_ = QPixmap();
  } else {
    // Stop an existing fade and start fading the other direction from the
    // same place.
    fade_timeline_->stop();
    fade_timeline_->setDirection(direction);
    fade_timeline_->resume();
  }

  state_ = visible ? FadingToOn : FadingToOff;
}

void MoodbarProxyStyle::FaderValueChanged(qreal value) { slider_->update(); }

bool MoodbarProxyStyle::eventFilter(QObject* object, QEvent* event) {
  if (object == slider_) {
    switch (event->type()) {
      case QEvent::Resize:
        // The widget was resized, we've got to render a new pixmap.
        moodbar_pixmap_dirty_ = true;
        break;

      case QEvent::ContextMenu:
        ShowContextMenu(static_cast<QContextMenuEvent*>(event)->globalPos());
        return true;

      default:
        break;
    }
  }

  return QProxyStyle::eventFilter(object, event);
}

void MoodbarProxyStyle::drawComplexControl(ComplexControl control,
                                           const QStyleOptionComplex* option,
                                           QPainter* painter,
                                           const QWidget* widget) const {
  if (control != CC_Slider || widget != slider_) {
    QProxyStyle::drawComplexControl(control, option, painter, widget);
    return;
  }

  const_cast<MoodbarProxyStyle*>(this)
      ->Render(control, qstyleoption_cast<const QStyleOptionSlider*>(option),
               painter, widget);
}

void MoodbarProxyStyle::Render(ComplexControl control,
                               const QStyleOptionSlider* option,
                               QPainter* painter, const QWidget* widget) {
  const qreal fade_value = fade_timeline_->currentValue();

  // Have we finished fading?
  if (state_ == FadingToOn && fade_value == 1.0) {
    state_ = MoodbarOn;
  } else if (state_ == FadingToOff && fade_value == 0.0) {
    state_ = MoodbarOff;
  }

  switch (state_) {
    case FadingToOn:
    case FadingToOff:
      // Update the cached pixmaps if necessary
      if (fade_source_.isNull()) {
        // Draw the normal slider into the fade source pixmap.
        fade_source_ = QPixmap(option->rect.size());
        fade_source_.fill(
            option->palette.color(QPalette::Active, QPalette::Background));

        QPainter p(&fade_source_);
        QStyleOptionSlider opt_copy(*option);
        opt_copy.rect.moveTo(0, 0);

        QProxyStyle::drawComplexControl(control, &opt_copy, &p, widget);

        p.end();
      }

      if (fade_target_.isNull()) {
        if (state_ == FadingToOn) {
          EnsureMoodbarRendered(option);
        }
        fade_target_ = moodbar_pixmap_;
        QPainter p(&fade_target_);
        DrawArrow(option, &p);
        p.end();
      }

      // Blend the pixmaps into each other
      painter->drawPixmap(option->rect, fade_source_);
      painter->setOpacity(fade_value);
      painter->drawPixmap(option->rect, fade_target_);
      painter->setOpacity(1.0);
      break;

    case MoodbarOff:
      // It's a normal slider widget.
      QProxyStyle::drawComplexControl(control, option, painter, widget);
      break;

    case MoodbarOn:
      EnsureMoodbarRendered(option);
      painter->drawPixmap(option->rect, moodbar_pixmap_);
      DrawArrow(option, painter);
      break;
  }
}

void MoodbarProxyStyle::EnsureMoodbarRendered(const QStyleOptionSlider* opt) {
  if (moodbar_colors_dirty_) {
    moodbar_colors_ =
        MoodbarRenderer::Colors(data_, moodbar_style_, slider_->palette());
    moodbar_colors_dirty_ = false;
    moodbar_pixmap_dirty_ = true;
  }

  if (moodbar_pixmap_dirty_) {
    moodbar_pixmap_ = MoodbarPixmap(moodbar_colors_, slider_->size(),
                                    slider_->palette(), opt);
    moodbar_pixmap_dirty_ = false;
  }
}

int MoodbarProxyStyle::GetExtraSpace(const QStyleOptionComplex* opt) const {
  int space_available = slider_->style()->pixelMetric(
      QStyle::PM_SliderSpaceAvailable, opt, slider_);
  int w = slider_->width();
  return w - space_available;
}

QRect MoodbarProxyStyle::subControlRect(ComplexControl cc,
                                        const QStyleOptionComplex* opt,
                                        SubControl sc,
                                        const QWidget* widget) const {
  if (cc != QStyle::CC_Slider || widget != slider_) {
    return QProxyStyle::subControlRect(cc, opt, sc, widget);
  }

  switch (state_) {
    case MoodbarOff:
    case FadingToOff:
      break;

    case MoodbarOn:
    case FadingToOn:
      switch (sc) {
        case SC_SliderGroove: {
          int margin_leftright = GetExtraSpace(opt) / 2;
          return opt->rect.adjusted(margin_leftright, kMarginSize,
                                    -margin_leftright, -kMarginSize);
        }
        case SC_SliderHandle: {
          const QStyleOptionSlider* slider_opt =
              qstyleoption_cast<const QStyleOptionSlider*>(opt);

          int space_available = slider_->style()->pixelMetric(
              QStyle::PM_SliderSpaceAvailable, opt, slider_);
          int w = slider_->width();
          int margin = (w - space_available) / 2;
          int x = 0;

          if (slider_opt->maximum != slider_opt->minimum) {
            x = (slider_opt->sliderValue - slider_opt->minimum) *
                (space_available - kArrowWidth) /
                (slider_opt->maximum - slider_opt->minimum);
          }

          x += margin;

          return QRect(QPoint(opt->rect.left() + x, opt->rect.top()),
                       QSize(kArrowWidth, kArrowHeight));
        }

        default:
          break;
      }
  }

  return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

void MoodbarProxyStyle::DrawArrow(const QStyleOptionSlider* option,
                                  QPainter* painter) const {
  // Get the dimensions of the arrow
  const QRect rect =
      subControlRect(CC_Slider, option, SC_SliderHandle, slider_);

  // Make a polygon
  QPolygon poly;
  poly << rect.topLeft() << rect.topRight()
       << QPoint(rect.center().x(), rect.bottom());

  // Draw it
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->translate(0.5, 0.5);
  painter->setPen(Qt::black);
  painter->setBrush(slider_->palette().brush(QPalette::Active, QPalette::Base));
  painter->drawPolygon(poly);
  painter->restore();
}

QPixmap MoodbarProxyStyle::MoodbarPixmap(const ColorVector& colors,
                                         const QSize& size,
                                         const QPalette& palette,
                                         const QStyleOptionSlider* opt) {
  int margin_leftright = GetExtraSpace(opt);
  const QRect rect(QPoint(0, 0), size);
  QRect border_rect(rect);
  // I would expect we need to adjust by margin_lr/2, so the extra space is
  // distributed on both side, but if we do so, the margin is too small, and I'm
  // not sure why...
  border_rect.adjust(margin_leftright, kMarginSize, -margin_leftright,
                     -kMarginSize);

  QRect inner_rect(border_rect);
  inner_rect.adjust(kBorderSize, kBorderSize, -kBorderSize, -kBorderSize);

  QPixmap ret(size);
  QPainter p(&ret);

  // Draw the moodbar
  MoodbarRenderer::Render(colors, &p, inner_rect);

  // Draw the border
  p.setPen(
      QPen(Qt::black, kBorderSize, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
  p.drawRect(border_rect.adjusted(0, 0, -1, -1));

  // Draw the outer bit
  p.setPen(QPen(palette.brush(QPalette::Active, QPalette::Background),
                kMarginSize, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
  // First: a rectangle around the slider
  p.drawRect(rect.adjusted(1, 1, -2, -2));
  // Then, thicker border on left and right, because of the margins.
  p.setPen(QPen(palette.brush(QPalette::Active, QPalette::Background),
                margin_leftright * 2 - kBorderSize, Qt::SolidLine, Qt::FlatCap,
                Qt::MiterJoin));
  p.drawLine(rect.topLeft(), rect.bottomLeft());
  p.drawLine(rect.topRight(), rect.bottomRight());

  p.end();

  return ret;
}

void MoodbarProxyStyle::ShowContextMenu(const QPoint& pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu(slider_);
    show_moodbar_action_ = context_menu_->addAction(
        tr("Show moodbar"), this, SLOT(SetMoodbarEnabled(bool)));

    show_moodbar_action_->setCheckable(true);
    show_moodbar_action_->setChecked(enabled_);

    QMenu* styles_menu = context_menu_->addMenu(tr("Moodbar style"));
    style_action_group_ = new QActionGroup(styles_menu);

    for (int i = 0; i < MoodbarRenderer::StyleCount; ++i) {
      const MoodbarRenderer::MoodbarStyle style =
          MoodbarRenderer::MoodbarStyle(i);

      QAction* action =
          style_action_group_->addAction(MoodbarRenderer::StyleName(style));
      action->setCheckable(true);
      action->setData(i);
    }

    styles_menu->addActions(style_action_group_->actions());

    connect(styles_menu, SIGNAL(triggered(QAction*)),
            SLOT(ChangeStyle(QAction*)));
  }

  // Update the currently selected style
  for (QAction* action : style_action_group_->actions()) {
    if (MoodbarRenderer::MoodbarStyle(action->data().toInt()) ==
        moodbar_style_) {
      action->setChecked(true);
      break;
    }
  }

  context_menu_->popup(pos);
}

void MoodbarProxyStyle::ChangeStyle(QAction* action) {
  QSettings s;
  s.beginGroup("Moodbar");
  s.setValue("style", action->data().toInt());

  app_->ReloadSettings();
}
