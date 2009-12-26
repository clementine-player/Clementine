#include "radioitem.h"

RadioItem::RadioItem(RadioService* _service, int type, const QString& key,
                     RadioItem* parent)
  : SimpleTreeItem<RadioItem>(type, key, parent),
    service(_service),
    playable(false)
{
}
