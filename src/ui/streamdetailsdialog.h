#ifndef STREAMDETAILSDIALOG_H
#define STREAMDETAILSDIALOG_H

#include <memory>

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
  void setBitrate(int);
  void setDepth(int);
  void setChannels(int);
  void setSampleRate(int);

 private slots:
  void Close();

 private:
  std::unique_ptr<Ui::StreamDetailsDialog> ui_;
};

#endif  // STREAMDETAILSDIALOG_H
