#ifndef RADIOVIEW_H
#define RADIOVIEW_H

#include <QTreeView>

class RadioView : public QTreeView {
  Q_OBJECT

 public:
  RadioView(QWidget* parent = 0);

  // QWidget
  void contextMenuEvent(QContextMenuEvent* e);
};

#endif // RADIOVIEW_H
