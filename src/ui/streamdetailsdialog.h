#ifndef STREAMDETAILSDIALOG_H
#define STREAMDETAILSDIALOG_H

#include <QDialog>

namespace Ui {
class StreamDetailsDialog;
}

class StreamDetailsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit StreamDetailsDialog(QWidget* parent = 0);
  ~StreamDetailsDialog();

  void setUrl(const QString& url);
  void setFormat(const QString& codec);  // This is localized, so only for human
                                         // consumption.
  void setBitrate(unsigned int);
  void setDepth(unsigned int);
  void setChannels(unsigned int);
  void setSampleRate(unsigned int);

 private slots:
  void Close();

 private:
  Ui::StreamDetailsDialog* ui_;
};

#endif  // STREAMDETAILSDIALOG_H
