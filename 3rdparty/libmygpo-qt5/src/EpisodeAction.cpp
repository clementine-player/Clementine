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

#include "EpisodeAction_p.h"

#include "qjsonwrapper/Json.h"

using namespace mygpo;

static qulonglong c_maxlonglong = (2^64)-1;

EpisodeActionPrivate::EpisodeActionPrivate( EpisodeAction* qq, const QVariant& variant, QObject* parent ) : QObject( parent ), q( qq )
{
    bool valid = parse( variant );
    qq->setProperty("valid", QVariant( valid ) );
}

EpisodeActionPrivate::EpisodeActionPrivate( EpisodeAction* qq, const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, qulonglong started, qulonglong position, qulonglong total, QObject* parent )
    : QObject( parent ), q( qq ), m_podcastUrl( podcastUrl ), m_episodeUrl( episodeUrl ), m_deviceName( deviceName ), m_action( action ), m_timestamp( timestamp ), m_started( started ), m_position( position ), m_total( total )
{

}

EpisodeActionPrivate::~EpisodeActionPrivate()
{

}

bool EpisodeActionPrivate::parse( const QVariant& data )
{
    if( !data.canConvert( QVariant::Map ) )
        return false;
    QVariantMap episodeActionMap = data.toMap();

    QVariant s = episodeActionMap.value( QLatin1String( "podcast" ) );
    if( !s.canConvert( QVariant::Url ) )
        return false;
    m_podcastUrl = s.toUrl();

    s = episodeActionMap.value( QLatin1String( "episode" ) );
    if( !s.canConvert( QVariant::Url ) )
        return false;
    m_episodeUrl = s.toUrl();

    if( episodeActionMap.contains( QLatin1String( "device" ) ) )
    {
        s = episodeActionMap.value( QLatin1String( "device" ) );
        if( !s.canConvert( QVariant::String ) )
            return false;
        m_deviceName = s.toString();
    }
    else
    {
        m_deviceName = QLatin1String( "" );
    }

    s = episodeActionMap.value( QLatin1String( "action" ) );
    if( !s.canConvert( QVariant::String ) )
        return false;
    if( !parseActionType( s.toString() ) )
        return false;

    if( episodeActionMap.contains( QLatin1String( "started" ) ) )
    {
        s = episodeActionMap.value( QLatin1String( "started" ) );
        if( !s.canConvert( QVariant::ULongLong ) )
            return false;
        m_started = s.toULongLong();
    }
    else
    {
        m_started = 0;
    }

    if( episodeActionMap.contains( QLatin1String( "position" ) ) )
    {
        s = episodeActionMap.value( QLatin1String( "position" ) );
        if( !s.canConvert( QVariant::ULongLong ) )
            return false;
        m_position = s.toULongLong();
    }
    else
    {
        m_position = 0;
    }

    if( episodeActionMap.contains( QLatin1String( "total" ) ) )
    {
        s = episodeActionMap.value( QLatin1String( "total" ) );
        if( !s.canConvert( QVariant::ULongLong ) )
            return false;
        m_total = s.toULongLong();
    }
    else
    {
        m_total = 0;
    }

    if( episodeActionMap.contains( QLatin1String( "timestamp" ) ) )
    {
        s = episodeActionMap.value( QLatin1String( "timestamp" ) );
        m_timestamp = s.toULongLong();
    }
    else
    {
        m_timestamp = 0;
    }

    return true;
}

bool EpisodeActionPrivate::parseActionType( const QString& data )
{
    if( data.compare( QLatin1String( "delete" ) ) == 0 )
    {
        m_action = EpisodeAction::Delete;
        return true;
    }
    else if( data.compare( QLatin1String( "download" ) ) == 0 )
    {
        m_action = EpisodeAction::Download;
        return true;
    }
    else if( data.compare( QLatin1String( "play" ) ) == 0 )
    {
        m_action = EpisodeAction::Play;
        return true;
    }
    else if( data.compare( QLatin1String( "new" ) ) == 0 )
    {
        m_action = EpisodeAction::New;
        return true;
    }
    else if ( data.compare( QLatin1String( "flattr" ) ) == 0 )
    {
        m_action = EpisodeAction::Flattr;
        return true;
    }
    else
    {
        return false;
    }
}

bool EpisodeActionPrivate::parse( const QByteArray& data )
{
    bool ok;
    QVariant variant = QJsonWrapper::parseJson( data, &ok );
    if( ok )
    {
        if( !parse( variant ) ) return false;
        return true;
    }
    else
    {
        return false;
    }
}

QUrl EpisodeActionPrivate::podcastUrl() const
{
    return m_podcastUrl;
}

QUrl EpisodeActionPrivate::episodeUrl() const
{
    return m_episodeUrl;
}

QString EpisodeActionPrivate::deviceName() const
{
    return m_deviceName;
}

EpisodeAction::ActionType EpisodeActionPrivate::action() const
{
    return m_action;
}

qulonglong EpisodeActionPrivate::timestamp() const
{
    return m_timestamp;
}

qulonglong EpisodeActionPrivate::started() const
{
    return m_started;
}

qulonglong EpisodeActionPrivate::position() const
{
    return m_position;
}

qulonglong EpisodeActionPrivate::total() const
{
    return m_total;
}

// ### End of EpisodeActionPrivate

EpisodeAction::EpisodeAction( const QVariant& variant, QObject* parent ): QObject( parent ), d( new EpisodeActionPrivate( this, variant ) )
{

}

EpisodeAction::EpisodeAction( const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, qulonglong started, qulonglong position, qulonglong total, QObject* parent )
    : QObject( parent ), d( new EpisodeActionPrivate( this, podcastUrl, episodeUrl, deviceName, action, timestamp, started, position, total ) )
{

}

EpisodeAction::EpisodeAction(const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, qulonglong position, QObject* parent)
    : QObject( parent ), d( new EpisodeActionPrivate( this, podcastUrl, episodeUrl, deviceName, action, timestamp, c_maxlonglong, position, c_maxlonglong ) )
{

}

EpisodeAction::EpisodeAction(const QUrl& podcastUrl, const QUrl& episodeUrl, const QString& deviceName, EpisodeAction::ActionType action, qulonglong timestamp, QObject* parent)
    : QObject( parent ), d( new EpisodeActionPrivate( this, podcastUrl, episodeUrl, deviceName, action, timestamp, c_maxlonglong, c_maxlonglong, c_maxlonglong ) )
{

}

EpisodeAction::~EpisodeAction()
{
    delete d;
}

QUrl EpisodeAction::podcastUrl() const
{
    return d->podcastUrl();
}

QUrl EpisodeAction::episodeUrl() const
{
    return d->episodeUrl();
}

QString EpisodeAction::deviceName() const
{
    return d->deviceName();
}

EpisodeAction::ActionType EpisodeAction::action() const
{
    return d->action();
}

qulonglong EpisodeAction::timestamp() const
{
    return d->timestamp();
}

qulonglong EpisodeAction::started() const
{
    return d->started();
}

qulonglong EpisodeAction::position() const
{
    return d->position();
}

qulonglong EpisodeAction::total() const
{
    return d->total();
}
