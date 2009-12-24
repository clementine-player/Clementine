#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QWidget>

#include "ui_fileview.h"

class QFileSystemModel;
class QUndoStack;

class FileView : public QWidget {
  Q_OBJECT

 public:
  FileView(QWidget* parent = 0);

  void SetPath(const QString& path);

 signals:
  void PathChanged(const QString& path);

  void PlayFile(const QString& path);
  void PlayDirectory(const QString& path);

 private slots:
  void FileUp();
  void FileBack();
  void FileForward();
  void FileHome();
  void ChangeFilePath(const QString& new_path);
  void ItemActivated(const QModelIndex& index);
  void ItemDoubleClick(const QModelIndex& index);

 private:
  void ChangeFilePathWithoutUndo(const QString& new_path);

 private:
  Ui::FileView ui_;

  QFileSystemModel* model_;
  QUndoStack* undo_stack_;
};

#endif // FILEVIEW_H
