#ifndef TRACKSLIDERPOPUP_H
#define TRACKSLIDERPOPUP_H

#include <QWidget>

class TrackSliderPopup : public QWidget {
  Q_OBJECT

public:
  TrackSliderPopup(QWidget* parent);

public slots:
  static bool IsTransparencyAvailable();

  void SetText(const QString& text);
  void SetPopupPosition(const QPoint& pos);

  void SetMouseOverSlider(bool mouse_over_slider);

protected:
  void paintEvent(QPaintEvent*);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void wheelEvent(QWheelEvent*);

private:
  static const int kTextMargin;
  static const int kPointLength;
  static const int kPointWidth;
  static const int kBorderRadius;
  static const qreal kBlurRadius;

  void UpdatePixmap();
  void UpdatePosition();
  void SendMouseEventToParent(QMouseEvent* e);

private slots:
  void UpdateVisibility();

private:
  QString text_;
  QPoint pos_;

  QFont font_;
  QFontMetrics font_metrics_;
  QPixmap pixmap_;
  QPixmap background_cache_;

  bool mouse_over_slider_;
  bool mouse_over_popup_;
  QTimer* visibility_timer_;
};

#endif // TRACKSLIDERPOPUP_H
