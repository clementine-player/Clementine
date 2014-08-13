#include "playlistsaveoptionsdialog.h"
#include "ui_playlistsaveoptionsdialog.h"

PlaylistSaveOptionsDialog::PlaylistSaveOptionsDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::PlaylistSaveOptionsDialog)
{
    ui->setupUi(this);
}

PlaylistSaveOptionsDialog::~PlaylistSaveOptionsDialog()
{
    delete ui;
}
