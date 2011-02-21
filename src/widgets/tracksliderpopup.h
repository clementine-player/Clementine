#ifndef TRACKSLIDERPOPUP_H
#define TRACKSLIDERPOPUP_H

#include <QWidget>

class TrackSliderPopup : public QWidget {
  Q_OBJECT

public:
  TrackSliderPopup(QWidget* parent);

public slots:
  void SetText(const QString& text);
  void SetSmallText(const QString& small_text);
  void SetPopupPosition(const QPoint& pos);

protected:
  void paintEvent(QPaintEvent*);

private:
  static const int kTextMargin;
  static const int kPointLength;
  static const int kPointWidth;
  static const int kBorderRadius;
  static const qreal kBlurRadius;

  void UpdatePixmap();
  void UpdatePosition();
  void SendMouseEventToParent(QMouseEvent* e);

private:
  QString text_;
  QString small_text_;
  QPoint pos_;

  QFont font_;
  QFont small_font_;
  QFontMetrics font_metrics_;
  QFontMetrics small_font_metrics_;
  QPixmap pixmap_;
  QPixmap background_cache_;
};

#endif // TRACKSLIDERPOPUP_H
