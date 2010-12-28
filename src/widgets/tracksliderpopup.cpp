#include "tracksliderpopup.h"

#include <QBitmap>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>
#include <QtDebug>

#ifdef Q_WS_X11
#  include <QX11Info>
#endif

const int TrackSliderPopup::kTextMargin = 4;
const int TrackSliderPopup::kPointLength = 16;
const int TrackSliderPopup::kPointWidth = 4;
const int TrackSliderPopup::kBorderRadius = 4;
const qreal TrackSliderPopup::kBlurRadius = 20.0;

void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);

TrackSliderPopup::TrackSliderPopup(QWidget* parent)
  : QWidget(parent),
    font_metrics_(fontMetrics()),
    mouse_over_slider_(false),
    mouse_over_popup_(false),
    visibility_timer_(new QTimer(this))
{
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_TranslucentBackground);
#ifndef Q_OS_DARWIN
  // Paints the whole rect with a white background on Mac.
  setAttribute(Qt::WA_OpaquePaintEvent);
  // Creates flicker on Mac.
  setAttribute(Qt::WA_TransparentForMouseEvents);
#endif
  setMouseTracking(true);

  visibility_timer_->setSingleShot(true);
  visibility_timer_->setInterval(10);
  connect(visibility_timer_, SIGNAL(timeout()), SLOT(UpdateVisibility()));

  font_.setPointSizeF(7.5);
  font_.setBold(true);
  font_metrics_ = QFontMetrics(font_);
}

bool TrackSliderPopup::IsTransparencyAvailable() {
#ifdef Q_WS_X11
  return QX11Info::isCompositingManagerRunning();
#endif
  return true;
}

void TrackSliderPopup::SetText(const QString& text) {
  text_ = text;
  UpdatePixmap();
}

void TrackSliderPopup::SetPopupPosition(const QPoint& pos) {
  pos_ = pos;
  UpdatePosition();
}

void TrackSliderPopup::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.drawPixmap(0, 0, pixmap_);
}

void TrackSliderPopup::UpdatePixmap() {
  const QRect text_rect(kBlurRadius + kTextMargin, kBlurRadius + kTextMargin,
                        font_metrics_.width(text_) + 2, font_metrics_.height());
  const int bubble_bottom = text_rect.bottom() + kTextMargin;
  const QRect total_rect(0, 0, text_rect.right() + kBlurRadius + kTextMargin,
                         kBlurRadius + bubble_bottom + kPointLength);
  const QRect bubble_rect(kBlurRadius, kBlurRadius,
                          total_rect.width() - kBlurRadius * 2,
                          bubble_bottom - kBlurRadius);

  if (background_cache_.size() != total_rect.size()) {
    const QColor highlight(palette().color(QPalette::Active, QPalette::Highlight));
    const QColor bg_color_1(highlight.lighter(110));
    const QColor bg_color_2(highlight.darker(120));

    QPolygon pointy;
    pointy << QPoint(total_rect.width()/2 - kPointWidth, bubble_bottom)
           << QPoint(total_rect.width()/2, total_rect.bottom() - kBlurRadius)
           << QPoint(total_rect.width()/2 + kPointWidth, bubble_bottom);

    QPolygon inner_pointy;
    inner_pointy << QPoint(pointy[0].x() + 1, pointy[0].y() - 1)
                 << QPoint(pointy[1].x(), pointy[1].y() - 1)
                 << QPoint(pointy[2].x() - 1, pointy[2].y() - 1);

    background_cache_ = QPixmap(total_rect.size());
    background_cache_.fill(IsTransparencyAvailable() ?
                           Qt::transparent : bg_color_2);
    QPainter p(&background_cache_);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);

    if (IsTransparencyAvailable()) {
      // Draw the shadow to a different image
      QImage blur_source(total_rect.size(), QImage::Format_ARGB32_Premultiplied);
      blur_source.fill(Qt::transparent);

      QPainter blur_painter(&blur_source);
      blur_painter.setRenderHint(QPainter::Antialiasing);
      blur_painter.setRenderHint(QPainter::HighQualityAntialiasing);
      blur_painter.setBrush(bg_color_2);
      blur_painter.drawRoundedRect(bubble_rect, kBorderRadius, kBorderRadius);
      blur_painter.drawPolygon(pointy);

      // Fade the shadow out towards the bottom
      QLinearGradient fade_gradient(QPoint(0, bubble_bottom),
                                    QPoint(0, bubble_bottom + kPointLength));
      fade_gradient.setColorAt(0.0, QColor(255, 0, 0, 0));
      fade_gradient.setColorAt(1.0, QColor(255, 0, 0, 255));
      blur_painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
      blur_painter.fillRect(total_rect, fade_gradient);
      blur_painter.end();

      p.save();
      qt_blurImage(&p, blur_source, kBlurRadius, true, false);
      p.restore();
    } else {
      QBitmap mask(total_rect.size());
      mask.clear();
      QPainter mask_painter(&mask);

      mask_painter.setBrush(Qt::color1);
      mask_painter.drawRoundedRect(bubble_rect, kBorderRadius, kBorderRadius);
      mask_painter.drawPolygon(pointy);
      mask_painter.end();

      setMask(mask);
    }

    // Outer bubble
    p.setPen(Qt::NoPen);
    p.setBrush(bg_color_2);
    p.drawRoundedRect(bubble_rect, kBorderRadius, kBorderRadius);

    // Outer pointy
    p.drawPolygon(pointy);

    // Inner bubble
    p.setBrush(bg_color_1);
    p.drawRoundedRect(bubble_rect.adjusted(1, 1, -1, -1),
                      kBorderRadius, kBorderRadius);

    // Inner pointy
    p.drawPolygon(inner_pointy);
  }

  pixmap_ = QPixmap(total_rect.size());
  pixmap_.fill(Qt::transparent);
  QPainter p(&pixmap_);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::HighQualityAntialiasing);

  // Background
  p.drawPixmap(total_rect.topLeft(), background_cache_);

  // Text
  p.setPen(palette().color(QPalette::Text));
  p.setFont(font_);
  p.drawText(text_rect, Qt::AlignHCenter, text_);
  p.end();

  resize(pixmap_.size());
  UpdatePosition();
  update();
}

void TrackSliderPopup::UpdatePosition() {
  move(pos_.x() - pixmap_.width() / 2,
       pos_.y() - pixmap_.height() + kBlurRadius);
}

void TrackSliderPopup::enterEvent(QEvent* e) {
  mouse_over_popup_ = true;
  visibility_timer_->start();
}

void TrackSliderPopup::leaveEvent(QEvent* e) {
  mouse_over_popup_ = false;
  visibility_timer_->start();
}

void TrackSliderPopup::SetMouseOverSlider(bool mouse_over_slider) {
  mouse_over_slider_ = mouse_over_slider;
  visibility_timer_->start();
}

void TrackSliderPopup::UpdateVisibility() {
  setVisible(mouse_over_popup_ || mouse_over_slider_);
}

void TrackSliderPopup::SendMouseEventToParent(QMouseEvent* e) {
  QMouseEvent event(
      e->type(), parentWidget()->mapFromGlobal(e->globalPos()),
      e->button(), e->buttons(), e->modifiers());
  QCoreApplication::sendEvent(parentWidget(), &event);
}

void TrackSliderPopup::wheelEvent(QWheelEvent* e) {
  QWheelEvent event(
      parentWidget()->mapFromGlobal(e->globalPos()), e->delta(), e->buttons(),
      e->modifiers(), e->orientation());
  QCoreApplication::sendEvent(parentWidget(), &event);
}

void TrackSliderPopup::mousePressEvent(QMouseEvent* e) {
  SendMouseEventToParent(e);
}

void TrackSliderPopup::mouseReleaseEvent(QMouseEvent* e) {
  SendMouseEventToParent(e);
}

void TrackSliderPopup::mouseMoveEvent(QMouseEvent* e) {
  if (!parentWidget()->rect().contains(
      parentWidget()->mapFromGlobal(e->globalPos()))) {
    // The mouse left the parent widget - close this popup
    mouse_over_popup_ = false;
    visibility_timer_->start();
  }
  SendMouseEventToParent(e);
}
