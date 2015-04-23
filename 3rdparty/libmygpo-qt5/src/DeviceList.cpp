/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2010 - 2011 Stefan Derkits <stefan@derkits.at>             *
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

#include "DeviceList_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

DeviceListPrivate::DeviceListPrivate( DeviceList* qq, QNetworkReply* reply ) : q( qq ), m_reply( reply ), m_error( QNetworkReply::NoError )
{
    QObject::connect( m_reply, SIGNAL( finished() ), this, SLOT( parseData() ) );
    QObject::connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( error( QNetworkReply::NetworkError ) ) );
}

DeviceListPrivate::~DeviceListPrivate()
{
}

QVariant DeviceListPrivate::devices() const
{
    return m_devices;
}

QList< DevicePtr > DeviceListPrivate::devicesList() const
{
    return m_devicesList;
}

void DeviceListPrivate::error( QNetworkReply::NetworkError error )
{
    m_error = error;
    emit q->requestError( error );
}

bool DeviceListPrivate::parse( const QVariant& data )
{
    if( !data.canConvert( QVariant::List ) )
        return false;

    QVariantList varList = data.toList();
    QVariantList devList;
    foreach( const QVariant & var, varList )
    {
        DevicePtr ptr( new Device( var, this ) );
        m_devicesList.append( ptr );
        QVariant v;
        v.setValue<DevicePtr>( ptr );
        devList.append( v );
    }
    m_devices = devList;
    return true;
}

bool DeviceListPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if( ok )
    {
        ok = ( parse( variant ) );
    }
    return ok;
}


void DeviceListPrivate::parseData()
{
    if( m_reply->error() == QNetworkReply::NoError )
    {
        if( parse( m_reply->readAll() ) )
        {
            emit q->finished();
        }
        else
        {
            emit q->parseError();
        }
    }
    m_reply->deleteLater();
}

DeviceList::DeviceList( QNetworkReply* reply, QObject* parent ) : QObject( parent ), d( new DeviceListPrivate( this, reply ) )
{

}

DeviceList::~DeviceList()
{
    delete d;
}

QVariant mygpo::DeviceList::devices() const
{
    return d->devices();
}

QList< DevicePtr > DeviceList::devicesList() const
{
    return d->devicesList();
}
