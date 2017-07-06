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

#ifndef DEVICE_PRIVATE_H
#define DEVICE_PRIVATE_H

#include "Device.h"

namespace mygpo
{

class DevicePrivate : public QObject
{
    Q_OBJECT

public:
    DevicePrivate( const QVariant& var );
    QString id() const;
    QString caption() const;
    QString type() const;
    qulonglong subscriptions() const;
private:
    QString m_id;
    QString m_caption;
    QString m_type;
    qulonglong m_subscriptions;
    bool parse( const QVariant& var );
};

}

#endif //DEVICE_PRIVATE_H
