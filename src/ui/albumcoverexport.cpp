/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "albumcoverexport.h"

#include <QSettings>

#include "ui_albumcoverexport.h"

const char* AlbumCoverExport::kSettingsGroup = "AlbumCoverExport";

AlbumCoverExport::AlbumCoverExport(QWidget* parent)
    : QDialog(parent), ui_(new Ui_AlbumCoverExport) {
  ui_->setupUi(this);

  connect(ui_->forceSize, SIGNAL(stateChanged(int)),
          SLOT(ForceSizeToggled(int)));
}

AlbumCoverExport::~AlbumCoverExport() { delete ui_; }

AlbumCoverExport::DialogResult AlbumCoverExport::Exec() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // restore last accepted settings
  ui_->fileName->setText(s.value("fileName", "cover").toString());
  ui_->doNotOverwrite->setChecked(
      s.value("overwrite", OverwriteMode_None).toInt() == OverwriteMode_None);
  ui_->overwriteAll->setChecked(
      s.value("overwrite", OverwriteMode_All).toInt() == OverwriteMode_All);
  ui_->overwriteSmaller->setChecked(
      s.value("overwrite", OverwriteMode_Smaller).toInt() ==
      OverwriteMode_Smaller);
  ui_->forceSize->setChecked(s.value("forceSize", false).toBool());
  ui_->width->setText(s.value("width", "").toString());
  ui_->height->setText(s.value("height", "").toString());
  ui_->export_downloaded->setChecked(
      s.value("export_downloaded", true).toBool());
  ui_->export_embedded->setChecked(s.value("export_embedded", false).toBool());

  ForceSizeToggled(ui_->forceSize->checkState());

  DialogResult result = DialogResult();
  result.cancelled_ = (exec() == QDialog::Rejected);

  if (!result.cancelled_) {
    QString fileName = ui_->fileName->text();
    if (fileName.isEmpty()) {
      fileName = "cover";
    }
    OverwriteMode overwrite =
        ui_->doNotOverwrite->isChecked()
            ? OverwriteMode_None
            : (ui_->overwriteAll->isChecked() ? OverwriteMode_All
                                              : OverwriteMode_Smaller);
    bool forceSize = ui_->forceSize->isChecked();
    QString width = ui_->width->text();
    QString height = ui_->height->text();

    s.setValue("fileName", fileName);
    s.setValue("overwrite", overwrite);
    s.setValue("forceSize", forceSize);
    s.setValue("width", width);
    s.setValue("height", height);
    s.setValue("export_downloaded", ui_->export_downloaded->isChecked());
    s.setValue("export_embedded", ui_->export_embedded->isChecked());

    result.fileName_ = fileName;
    result.overwrite_ = overwrite;
    result.forceSize_ = forceSize;
    result.width_ = width.toInt();
    result.height_ = height.toInt();
    result.export_downloaded_ = ui_->export_downloaded->isChecked();
    result.export_embedded_ = ui_->export_embedded->isChecked();
  }

  return result;
}

void AlbumCoverExport::ForceSizeToggled(int state) {
  ui_->width->setEnabled(state == Qt::Checked);
  ui_->height->setEnabled(state == Qt::Checked);
}
