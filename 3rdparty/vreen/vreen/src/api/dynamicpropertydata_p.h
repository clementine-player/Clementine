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
#ifndef VK_DYNAMICPROPERTYDATA_P_H
#define VK_DYNAMICPROPERTYDATA_P_H

//from euroelessar code

#include <QSharedData>
#include <QVariant>

namespace Vreen {

class DynamicPropertyData;

namespace CompiledProperty
{
    typedef QVariant (DynamicPropertyData::*Getter)() const;
    typedef void (DynamicPropertyData::*Setter)(const QVariant &variant);
}

class DynamicPropertyData : public QSharedData
{
public:
    typedef CompiledProperty::Getter Getter;
    typedef CompiledProperty::Setter Setter;
    DynamicPropertyData() {}
    DynamicPropertyData(const DynamicPropertyData &o) :
            QSharedData(o), names(o.names), values(o.values) {}
    QList<QByteArray> names;
    QList<QVariant> values;

    QVariant property(const char *name, const QVariant &def, const QList<QByteArray> &names,
                      const QList<Getter> &getters) const;
    void setProperty(const char *name, const QVariant &value, const QList<QByteArray> &names,
                     const QList<Setter> &setters);
};

} // namespace Vreen

#endif // VK_DYNAMICPROPERTYDATA_P_H

