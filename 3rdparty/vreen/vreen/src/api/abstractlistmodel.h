/****************************************************************************
**
** Vreen - vk.com API Qt bindings
**
** Copyright © 2012 Aleksey Sidorov <gorthauer87@ya.ru>
**
*****************************************************************************
**
** $VREEN_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $VREEN_END_LICENSE$
**
****************************************************************************/
#ifndef ABSTRACTLISTMODEL_H
#define ABSTRACTLISTMODEL_H

#include <QAbstractListModel>
#include "vk_global.h"

namespace Vreen {

class VK_SHARED_EXPORT AbstractListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AbstractListModel(QObject *parent = 0);
    Q_INVOKABLE QVariantMap get(int row);
    Q_INVOKABLE QVariant get(int row, const QByteArray &field);
};

} //namespace Vreen

#endif // ABSTRACTLISTMODEL_H

