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
#include "abstractlistmodel.h"
#include <QDebug>

namespace Vreen {

AbstractListModel::AbstractListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

QVariantMap AbstractListModel::get(int row)
{
    auto roles = roleNames();
    QVariantMap map;
    auto index = createIndex(row, 0);
    for (auto it = roles.constBegin(); it != roles.constEnd(); it++) {
        auto value = data(index, it.key());
        map.insert(it.value(), value);
    }
    return map;
}

QVariant AbstractListModel::get(int row, const QByteArray &field)
{
    auto index = createIndex(row, 0);
    return data(index, roleNames().key(field));
}

} //namespace Vreen

