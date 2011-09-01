#include "globalsearchpopup.h"

#include <QShortcut>

#include "ui/iconloader.h"

GlobalSearchPopup::GlobalSearchPopup(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_GlobalSearchPopup) {
  ui_->setupUi(this);

  Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Popup;
  setWindowFlags(flags);

  ui_->previous->setIcon(IconLoader::Load("media-skip-backward"));
  ui_->next->setIcon(IconLoader::Load("media-skip-forward"));
  ui_->play_pause->setIcon(IconLoader::Load("media-playback-start"));
  ui_->stop->setIcon(IconLoader::Load("media-playback-stop"));

  QShortcut* shortcut = new QShortcut(QKeySequence::Close, this);
  connect(shortcut, SIGNAL(activated()), SLOT(close()));
}

void GlobalSearchPopup::Init(LibraryBackendInterface* library) {
  ui_->search_widget->Init(library);

  // Forward AddToPlaylist signal.
  connect(ui_->search_widget, SIGNAL(AddToPlaylist(QMimeData*)),
      SIGNAL(AddToPlaylist(QMimeData*)));
}

void GlobalSearchPopup::setFocus(Qt::FocusReason reason) {
  ui_->search_widget->setFocus(reason);
}
