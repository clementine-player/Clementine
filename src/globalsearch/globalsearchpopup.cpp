#include "globalsearchpopup.h"

#include "ui/iconloader.h"

GlobalSearchPopup::GlobalSearchPopup(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_GlobalSearchPopup) {
  ui_->setupUi(this);

  ui_->previous->setIcon(IconLoader::Load("media-skip-backward"));
  ui_->next->setIcon(IconLoader::Load("media-skip-forward"));
  ui_->play_pause->setIcon(IconLoader::Load("media-playback-start"));
  ui_->stop->setIcon(IconLoader::Load("media-playback-stop"));
}

void GlobalSearchPopup::Init(LibraryBackendInterface* library) {
  ui_->search_widget->Init(library);

  // Forward AddToPlaylist signal.
  connect(ui_->search_widget, SIGNAL(AddToPlaylist(QMimeData*)),
      SIGNAL(AddToPlaylist(QMimeData*)));
}
