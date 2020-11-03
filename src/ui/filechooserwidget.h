#ifndef FILECHOOSERWIDGET_H
#define FILECHOOSERWIDGET_H

#include <QWidget>
class QLineEdit;
class QHBoxLayout;

class FileChooserWidget : public QWidget {
  Q_OBJECT
 public:
  enum class Mode { File, Directory };

 private:
  QHBoxLayout* layout_;
  QLineEdit* path_edit_;
  const Mode mode_;
  QString filter_;
  QString open_dir_path_;

 public:
  FileChooserWidget(QWidget* parent);
  FileChooserWidget(Mode mode, const QString& initial_path = "",
                    QWidget* parent = nullptr);
  FileChooserWidget(Mode mode, const QString& label,
                    const QString& initial_path = "",
                    QWidget* parent = nullptr);
  ~FileChooserWidget() = default;

  void SetFileFilter(const QString& filter);

  void SetPath(const QString& path);
  QString Path() const;

 public slots:
  void ChooseFile();

 private:
  void Init(const QString& initialPath);
};

#endif  // FILECHOOSERWIDGET_H
