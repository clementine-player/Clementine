/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2010 - 2013 Stefan Derkits <stefan@derkits.at>             *
* Copyright (c) 2010 - 2011 Christian Wagner <christian.wagner86@gmx.at>   *
* Copyright (c) 2010 - 2011 Felix Winter <ixos01@gmail.com>                *
*                                                                          *
* This library is free software; you can redistribute it and/or            *
* modify it under the terms of the GNU Lesser General Public               *
* License as published by the Free Software Foundation; either             *
* version 2.1 of the License, or (at your option) any later version.       *
*                                                                          *
* This library is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
* Lesser General Public License for more details.                          *
*                                                                          *
* You should have received a copy of the GNU Lesser General Public         *
* License along with this library; if not, write to the Free Software      *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 *
* USA                                                                      *
***************************************************************************/

#ifndef LIBMYGPO_QT_DEVICE_H
#define LIBMYGPO_QT_DEVICE_H

#include "mygpo_export.h"

#include <QSharedPointer>
#include <QVariant>

namespace mygpo
{
class DevicePrivate;
class MYGPO_EXPORT Device : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString id READ id CONSTANT )
    Q_PROPERTY( QString caption READ caption CONSTANT )
    Q_PROPERTY( QString type READ type CONSTANT )
    Q_PROPERTY( qulonglong subscriptions READ subscriptions CONSTANT )

public:

    enum Type
    {
        DESKTOP,
        LAPTOP,
        MOBILE,
        SERVER,
        OTHER
    };

    Device( const QVariant& var, QObject* parent = 0 );
    virtual ~Device();
    QString id() const;
    QString caption() const;
    QString type() const;
    qulonglong subscriptions() const;

private:
    Q_DISABLE_COPY( Device )
    DevicePrivate* const d;
};

typedef QSharedPointer<Device> DevicePtr;

}

Q_DECLARE_METATYPE(mygpo::DevicePtr)

#endif //LIBMYGPO_QT_DEVICE_H
