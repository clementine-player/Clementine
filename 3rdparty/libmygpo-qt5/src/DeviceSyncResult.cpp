/***************************************************************************
* This file is part of libmygpo-qt                                         *
* Copyright (c) 2011 - 2013 Stefan Derkits <stefan@derkits.at>             *
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

#include "DeviceSyncResult_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

DeviceSyncResultPrivate::DeviceSyncResultPrivate( DeviceSyncResult* qq, QNetworkReply* reply ) : q( qq ), m_reply( reply ), m_error( QNetworkReply::NoError )
{
    QObject::connect( m_reply, SIGNAL( finished() ), this, SLOT( parseData() ) );
    QObject::connect( m_reply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( error( QNetworkReply::NetworkError ) ) );
}

DeviceSyncResultPrivate::~DeviceSyncResultPrivate()
{

}

QVariant DeviceSyncResultPrivate::synchronized() const
{
    return m_synchronized;
}

QVariant DeviceSyncResultPrivate::notSynchronized() const
{
    return m_notSynchronized;
}

QList<QStringList> DeviceSyncResultPrivate::synchronizedList() const
{
    QVariantList synchronizedVarList = synchronized().toList();
    QList<QStringList> synchronizedList;
    foreach( const QVariant & list, synchronizedVarList )
    {
        QVariantList innerVarList = list.toList();
        QStringList innerList;
        foreach( const QVariant& device, innerVarList )
        {
            innerList.append(device.toString());
        }
        synchronizedList.append(innerList);
    }
    return synchronizedList;
}

QList<QString> DeviceSyncResultPrivate::notSynchronizedList() const
{
    QVariantList notSynchronizedVarList = notSynchronized().toList();
    QList<QString> notSynchronizedList;
    foreach ( const QVariant& device, notSynchronizedVarList )
    {
        notSynchronizedList.append(device.toString());
    }
    return notSynchronizedList;
}

bool DeviceSyncResultPrivate::parse( const QVariant& data )
{
    if( !data.canConvert( QVariant::Map ) )
        return false;
    QVariantMap varMap = data.toMap();
    m_synchronized = varMap.value( QLatin1String( "synchronized" ) );
    m_notSynchronized = varMap.value( QLatin1String( "not-synchronized" ) );
    return true;
}

bool DeviceSyncResultPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if( ok )
    {
        ok = ( parse( variant ) );
    }
    return ok;
}

void DeviceSyncResultPrivate::parseData()
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

void DeviceSyncResultPrivate::error( QNetworkReply::NetworkError error )
{
    m_error = error;
    emit q->requestError( error );
}

DeviceSyncResult::DeviceSyncResult ( QNetworkReply* reply, QObject* parent ) : QObject ( parent ), d( new DeviceSyncResultPrivate( this, reply ) )
{
    
}

DeviceSyncResult::~DeviceSyncResult()
{
    delete d;
}

QVariant DeviceSyncResult::synchronized() const
{
    return d->synchronized();
}

QVariant DeviceSyncResult::notSynchronized() const
{
    return d->notSynchronized();
}

QList<QStringList> DeviceSyncResult::synchronizedList() const
{
    return d->synchronizedList();
}

QList< QString > DeviceSyncResult::notSynchronizedList() const
{
    return d->notSynchronizedList();
}
