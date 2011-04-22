#ifndef SCRIPTDIALOGLIST_H
#define SCRIPTDIALOGLIST_H

#include <QListView>

class ScriptDialogList : public QListView {
public:
  ScriptDialogList(QWidget* parent = 0);

protected:
  void updateGeometries();
};

#endif // SCRIPTDIALOGLIST_H
