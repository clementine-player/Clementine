/* This file is part of Clementine.

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

#include "transcodedialog.h"
#include "transcoder.h"
#include "mainwindow.h"

#include <QPushButton>
#include <QFileDialog>
#include <QSettings>

// winspool.h defines this :(
#ifdef AddJob
#  undef AddJob
#endif

const char* TranscodeDialog::kSettingsGroup = "Transcoder";

static bool CompareFormatsByName(const TranscoderFormat* left,
                                 const TranscoderFormat* right) {
  return left->name() < right->name();
}


TranscodeDialog::TranscodeDialog(QWidget *parent)
  : QDialog(parent),
    transcoder_(new Transcoder(this))
{
  ui_.setupUi(this);
  ui_.files->header()->setResizeMode(QHeaderView::ResizeToContents);

  // Get formats
  QList<const TranscoderFormat*> formats = transcoder_->formats();
  qSort(formats.begin(), formats.end(), CompareFormatsByName);
  foreach (const TranscoderFormat* format, formats) {
    ui_.format->addItem(
        QString("%1 (.%2)").arg(format->name(), format->file_extension()),
        QVariant::fromValue(format));
  }

  // Load settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  last_add_dir_ = s.value("last_add_dir", QDir::homePath()).toString();

  // Add a start button
  start_button_ = ui_.button_box->addButton(
      tr("Start transcoding"), QDialogButtonBox::ActionRole);
  cancel_button_ = ui_.button_box->button(QDialogButtonBox::Cancel);
  close_button_ = ui_.button_box->button(QDialogButtonBox::Close);

  // Hide elements
  cancel_button_->hide();
  ui_.progress_group->hide();

  // Connect stuff
  connect(ui_.add, SIGNAL(clicked()), SLOT(Add()));
  connect(ui_.remove, SIGNAL(clicked()), SLOT(Remove()));
  connect(start_button_, SIGNAL(clicked()), SLOT(Start()));
  connect(cancel_button_, SIGNAL(clicked()), SLOT(Cancel()));
  connect(close_button_, SIGNAL(clicked()), SLOT(hide()));

  connect(transcoder_, SIGNAL(JobComplete(QString,bool)), SLOT(JobComplete(QString,bool)));
  connect(transcoder_, SIGNAL(AllJobsComplete()), SLOT(AllJobsComplete()));
}

void TranscodeDialog::SetWorking(bool working) {
  start_button_->setVisible(!working);
  cancel_button_->setVisible(working);
  close_button_->setVisible(!working);
  ui_.progress_group->setVisible(working);
  ui_.input_group->setEnabled(!working);
  ui_.output_group->setEnabled(!working);
}

void TranscodeDialog::Start() {
  SetWorking(true);

  QAbstractItemModel* file_model = ui_.files->model();
  const TranscoderFormat* format = ui_.format->itemData(
      ui_.format->currentIndex()).value<const TranscoderFormat*>();

  for (int i=0 ; i<file_model->rowCount() ; ++i) {
    QString filename = file_model->index(i, 0).data(Qt::UserRole).toString();
    transcoder_->AddJob(filename, format);
  }

  ui_.progress_bar->setValue(0);
  ui_.progress_bar->setMaximum(file_model->rowCount());

  progress_ = 0;
  errors_ = 0;
  transcoder_->Start();
}

void TranscodeDialog::Cancel() {
  SetWorking(false);
}

void TranscodeDialog::JobComplete(const QString& filename, bool success) {
  progress_ ++;
  if (!success) errors_ ++;

  ui_.progress_bar->setValue(progress_);
}

void TranscodeDialog::AllJobsComplete() {
  SetWorking(false);
}

void TranscodeDialog::Add() {
  QStringList filenames = QFileDialog::getOpenFileNames(
      this, tr("Add files to transcode"), last_add_dir_,
      QString("%1;;%2").arg(tr(MainWindow::kMusicFilterSpec),
                            tr(MainWindow::kAllFilesFilterSpec)));

  if (filenames.isEmpty())
    return;

  foreach (const QString& filename, filenames) {
    QString name = filename.section('/', -1, -1);
    QString path = filename.section('/', 0, -2);

    QTreeWidgetItem* item = new QTreeWidgetItem(
        ui_.files, QStringList() << name << path);
    item->setData(0, Qt::UserRole, filename);
  }

  last_add_dir_ = filenames[0];
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("last_add_dir", last_add_dir_);
}

void TranscodeDialog::Remove() {
  qDeleteAll(ui_.files->selectedItems());
}
