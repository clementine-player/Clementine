#ifndef GLOBALSEARCHPOPUP_H
#define GLOBALSEARCHPOPUP_H

#include <boost/scoped_ptr.hpp>

#include <QDialog>

#include "ui_globalsearchpopup.h"

class LibraryBackendInterface;

class GlobalSearchPopup : public QDialog {
  Q_OBJECT
 public:
  explicit GlobalSearchPopup(QWidget* parent = 0);

  void Init(LibraryBackendInterface* library);

 signals:
  void AddToPlaylist(QMimeData*);

 private:
  boost::scoped_ptr<Ui_GlobalSearchPopup> ui_;
};

#endif  // GLOBALSEARCHPOPUP_H
