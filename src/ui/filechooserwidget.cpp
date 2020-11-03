#include "filechooserwidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
FileChooserWidget::FileChooserWidget(QWidget* parent)
    : QWidget(parent),
      layout_(new QHBoxLayout(this)),
      path_edit_(new QLineEdit(this)),
      mode_(Mode::Directory),
      filter_(""),
      open_dir_path_("") {
  Init("");
}

FileChooserWidget::FileChooserWidget(Mode mode, const QString& initial_path,
                                     QWidget* parent)
    : QWidget(parent),
      layout_(new QHBoxLayout(this)),
      path_edit_(new QLineEdit(this)),
      mode_(mode),
      filter_(""),
      open_dir_path_("") {
  Init(initial_path);
}

FileChooserWidget::FileChooserWidget(Mode mode, const QString& label,
                                     const QString& initial_path,
                                     QWidget* parent)
    : QWidget(parent),
      layout_(new QHBoxLayout(this)),
      path_edit_(new QLineEdit(this)),
      mode_(mode),
      filter_(""),
      open_dir_path_("") {
  QLabel* lbl = new QLabel(label, this);
  layout_->addWidget(lbl);
  Init(initial_path);
}

void FileChooserWidget::SetFileFilter(const QString& filter) {
  filter_ = filter;
}

void FileChooserWidget::SetPath(const QString& path) {
  QFileInfo fi(path);
  if (fi.exists()) {
    path_edit_->setText(path);
    open_dir_path_ = fi.absolutePath();
  }
}

QString FileChooserWidget::Path() const {
  QString path(path_edit_->text());
  QFileInfo fi(path);
  if (!fi.exists()) return "";
  if (mode_ == Mode::File) {
    if (!fi.isFile()) return "";
  } else {
    if (!fi.isDir()) return "";
  }
  return path;
}

void FileChooserWidget::Init(const QString& initialPath) {
  QFileInfo fi(initialPath);
  if (fi.exists()) {
    path_edit_->setText(initialPath);
    open_dir_path_ = fi.absolutePath();
  }
  layout_->addWidget(path_edit_);

  QPushButton* changePath = new QPushButton(QLatin1String("..."), this);
  connect(changePath, &QAbstractButton::clicked, this,
          &FileChooserWidget::ChooseFile);
  changePath->setFixedWidth(
      2 * changePath->fontMetrics().width(QLatin1String(" ... ")));

  layout_->addWidget(changePath);
  layout_->setContentsMargins(2, 0, 2, 0);

  setFocusProxy(path_edit_);
}

void FileChooserWidget::ChooseFile() {
  QString new_path;

  if (mode_ == Mode::File)
    new_path = QFileDialog::getOpenFileName(this, tr("Select a file"),
                                            open_dir_path_, filter_);
  else
    new_path = QFileDialog::getExistingDirectory(this, tr("Select a directory"),
                                                 open_dir_path_);

  if (!new_path.isEmpty()) {
    QFileInfo fi(new_path);
    open_dir_path_ = fi.absolutePath();
    if (mode_ == Mode::File)
      path_edit_->setText(fi.absoluteFilePath());
    else
      path_edit_->setText(fi.absoluteFilePath() + "/");
  }
}
