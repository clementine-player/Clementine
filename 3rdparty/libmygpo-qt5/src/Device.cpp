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

#include "Device.h"

#include "Device_p.h"

using namespace mygpo;

Device::Device( const QVariant& var, QObject* parent ): QObject( parent ), d( new DevicePrivate( var ) )
{

}

Device::~Device()
{
    delete d;
}

QString Device::caption() const
{
    return d->caption();
}

QString Device::id() const
{
    return d->id();
}

qulonglong Device::subscriptions() const
{
    return d->subscriptions();
}

QString Device::type() const
{
    return d->type();
}


DevicePrivate::DevicePrivate( const QVariant& var ) : m_id(), m_caption(), m_type(), m_subscriptions( 0 )
{
    parse( var );
}

bool DevicePrivate::parse( const QVariant& var )
{
    if( var.canConvert( QVariant::Map ) )
    {
        QVariant vid, vcaption, vtype, vsubscriptions;
        QMap<QString, QVariant> varMap;
        varMap = var.toMap();
        vid = varMap.value( QLatin1String( "id" ) );
        vcaption = varMap.value( QLatin1String( "caption" ) );
        vtype = varMap.value( QLatin1String( "type" ) );
        vsubscriptions = varMap.value( QLatin1String( "subscriptions" ) );
        if( vid.canConvert( QVariant::String ) &&
                vcaption.canConvert( QVariant::String ) &&
                vtype.canConvert( QVariant::String ) &&
                vsubscriptions.canConvert( QVariant::LongLong ) )
        {
            m_id = vid.toString();
            m_caption = vcaption.toString();
            m_type = vtype.toString();
            m_subscriptions = vsubscriptions.toLongLong();
            return true;
        }
    }
    return false;
}


QString DevicePrivate::caption() const
{
    return this->m_caption;
}

QString DevicePrivate::id() const
{
    return this->m_id;
}

qulonglong DevicePrivate::subscriptions() const
{
    return this->m_subscriptions;
}

QString DevicePrivate::type() const
{
    return this->m_type;
}
