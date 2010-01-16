#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEdit : public QLineEdit {
  Q_OBJECT
  Q_PROPERTY(QString hint READ GetHint WRITE SetHint);

 public:
  LineEdit(QWidget* parent = 0);

  QString GetHint() const { return hint_; }
  void SetHint(const QString& hint);

  void paintEvent(QPaintEvent* e);

 private:
  QString hint_;
};

#endif // LINEEDIT_H
