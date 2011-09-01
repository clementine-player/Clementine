#include "globalsearchpopup.h"

#include <QShortcut>

#include "core/player.h"
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

void GlobalSearchPopup::Init(LibraryBackendInterface* library, Player* player) {
  ui_->search_widget->Init(library);

  // Forward AddToPlaylist signal.
  connect(ui_->search_widget, SIGNAL(AddToPlaylist(QMimeData*)),
      SIGNAL(AddToPlaylist(QMimeData*)));

  connect(ui_->previous, SIGNAL(clicked(bool)), player, SLOT(Previous()));
  connect(ui_->next, SIGNAL(clicked(bool)), player, SLOT(Next()));
  connect(ui_->play_pause, SIGNAL(clicked(bool)), player, SLOT(PlayPause()));
  connect(ui_->stop, SIGNAL(clicked(bool)), player, SLOT(Stop()));
}

void GlobalSearchPopup::setFocus(Qt::FocusReason reason) {
  ui_->search_widget->setFocus(reason);
}
