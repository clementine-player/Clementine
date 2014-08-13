#include "playlistsaveoptionsdialog.h"

#include "ui_playlistsaveoptionsdialog.h"
#include "playlistparsers/parserbase.h"

#include <QSettings>

const char* PlaylistSaveOptionsDialog::kSettingsGroup = "PlaylistSaveOptionsDialog";

PlaylistSaveOptionsDialog::PlaylistSaveOptionsDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::PlaylistSaveOptionsDialog)
{
  ui->setupUi(this);

  ui->filePaths->addItem(tr("Automatic"),PlaylistSaveOptions::Paths_Automatic);
  ui->filePaths->addItem(tr("Relative"), PlaylistSaveOptions::Paths_Relative);
  ui->filePaths->addItem(tr("Absolute"), PlaylistSaveOptions::Paths_Absolute);

  ui->pathSeparators->addItem(tr("Automatic"), PlaylistSaveOptions::Separators_Automatic);
  ui->pathSeparators->addItem(tr("Windows-style") + " (\\)", PlaylistSaveOptions::Separators_Windows);
  ui->pathSeparators->addItem(tr("Unix-style") + " (/)", PlaylistSaveOptions::Separators_Unix);

  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui->filePaths->setCurrentIndex(ui->filePaths->findData(
        s.value("file_paths", PlaylistSaveOptions::Paths_Automatic)));
  ui->pathSeparators->setCurrentIndex(ui->pathSeparators->findData(
        s.value("path_separators", PlaylistSaveOptions::Separators_Automatic)));
}

PlaylistSaveOptionsDialog::~PlaylistSaveOptionsDialog() { delete ui; }

void PlaylistSaveOptionsDialog::accept() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("file_paths", ui->filePaths->itemData(ui->filePaths->currentIndex()).toInt());
  s.setValue("path_separators", ui->pathSeparators->itemData(ui->pathSeparators->currentIndex()).toInt());

  QDialog::accept();
}

PlaylistSaveOptions PlaylistSaveOptionsDialog::options() const {
  PlaylistSaveOptions o;
  o.filePathStyle = static_cast<PlaylistSaveOptions::FilePathStyle>(
        ui->filePaths->itemData(ui->filePaths->currentIndex()).toInt());
  o.pathSeparatorStyle = static_cast<PlaylistSaveOptions::PathSeparatorStyle>(
        ui->pathSeparators->itemData(ui->pathSeparators->currentIndex()).toInt());
  return o;
}
