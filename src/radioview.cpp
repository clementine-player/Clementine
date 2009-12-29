#include "radioview.h"
#include "radiomodel.h"

#include <QContextMenuEvent>

RadioView::RadioView(QWidget *parent)
  : QTreeView(parent)
{
}

void RadioView::contextMenuEvent(QContextMenuEvent* e) {
  QModelIndex index = indexAt(e->pos());
  if (!index.isValid())
    return;

  RadioModel* radio_model = static_cast<RadioModel*>(model());
  radio_model->ShowContextMenu(radio_model->IndexToItem(index), e->globalPos());
}
