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
#include "dynamicpropertydata_p.h"
#include <QSharedData>

namespace Vreen {

QVariant DynamicPropertyData::property(const char *name, const QVariant &def,
                                       const QList<QByteArray> &gNames,
                                       const QList<Getter> &gGetters) const
{
    QByteArray prop = QByteArray::fromRawData(name, strlen(name));
    int id = gNames.indexOf(prop);
    if (id < 0) {
        id = names.indexOf(prop);
        if(id < 0)
            return def;
        return values.at(id);
    }
    return (this->*gGetters.at(id))();
}

void DynamicPropertyData::setProperty(const char *name, const QVariant &value,
                                      const QList<QByteArray> &gNames,
                                      const QList<Setter> &gSetters)
{
    QByteArray prop = QByteArray::fromRawData(name, strlen(name));
    int id = gNames.indexOf(prop);
    if (id < 0) {
        id = names.indexOf(prop);
        if (!value.isValid()) {
            if(id < 0)
                return;
            names.removeAt(id);
            values.removeAt(id);
        } else {
            if (id < 0) {
                prop.detach();
                names.append(prop);
                values.append(value);
            } else {
                values[id] = value;
            }
        }
    } else {
        (this->*gSetters.at(id))(value);
    }
}

} // namespace Vreen

