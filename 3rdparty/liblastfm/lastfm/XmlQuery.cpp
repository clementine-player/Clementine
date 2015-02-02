/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "XmlQuery.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QStringList>

using lastfm::XmlQuery;

class lastfm::XmlQueryPrivate
{
public:
    XmlQueryPrivate();
    QDomDocument domdoc;
    QDomElement e;
    lastfm::ws::ParseError error;
};


lastfm::XmlQueryPrivate::XmlQueryPrivate()
    : error( lastfm::ws::ParseError( lastfm::ws::NoError, "" ) )
{
}


XmlQuery::XmlQuery()
    : d( new XmlQueryPrivate )
{
}


XmlQuery::XmlQuery( const XmlQuery& that )
    : d( new XmlQueryPrivate( *that.d ) )
{
}

XmlQuery::XmlQuery( const QDomElement& e, const char* name )
    : d( new XmlQueryPrivate )
{
    d->e = e;
    if (e.isNull())
        qWarning() << "Expected node absent:" << name;
}


XmlQuery::~XmlQuery()
{
    delete d;
}


bool
XmlQuery::parse( const QByteArray& bytes )
{  
    if ( !bytes.size() )
        d->error = lastfm::ws::ParseError( lastfm::ws::MalformedResponse, "No data" );
    else
    {
        if( !d->domdoc.setContent( bytes ) )
            d->error = lastfm::ws::ParseError( lastfm::ws::MalformedResponse, "Invalid XML" );
        else
        {
            d->e = d->domdoc.documentElement();

            if (d->e.isNull())
                d->error = lastfm::ws::ParseError( lastfm::ws::MalformedResponse, "Lfm is null" );
            else
            {
                QString const status = d->e.attribute( "status" );
                QDomElement error = d->e.firstChildElement( "error" );
                uint const n = d->e.childNodes().count();

                // no elements beyond the lfm is perfectably acceptable for example when
                // XmlQuery is used parse response of a POST request.
                // if (n == 0) // nothing useful in the response
                if (status == "failed" || (n == 1 && !error.isNull()) )
                    d->error = error.isNull()
                            ? lastfm::ws::ParseError( lastfm::ws::MalformedResponse, "" )
                            : lastfm::ws::ParseError( lastfm::ws::Error( error.attribute( "code" ).toUInt() ), error.text() );
            }
        }
    }

    if ( d->error.enumValue() != lastfm::ws::NoError )
    {
        qDebug() << bytes;

        switch ( d->error.enumValue() )
        {
            case lastfm::ws::OperationFailed:
            case lastfm::ws::InvalidApiKey:
            case lastfm::ws::InvalidSessionKey:
                // NOTE will never be received during the LoginDialog stage
                // since that happens before this slot is registered with
                // QMetaObject in App::App(). Neat :)
                QMetaObject::invokeMethod( qApp, "onWsError", Q_ARG( lastfm::ws::Error, d->error.enumValue() ) );
                break;
            default:
                //do nothing
            break;
        }
    }

    return d->error.enumValue() == lastfm::ws::NoError;
}

bool
XmlQuery::parse( QNetworkReply* reply )
{
    reply->deleteLater();
    return parse( reply->readAll() ); 
}


lastfm::ws::ParseError
XmlQuery::parseError() const
{
    return d->error;
}


QString XmlQuery::text() const
{
    return d->e.text();
}


QString XmlQuery::attribute( const QString& name ) const
{
    return d->e.attribute( name );
}


XmlQuery
XmlQuery::operator[]( const QString& name ) const
{
    QStringList parts = name.split( ' ' );
    if (parts.size() >= 2)
    {
        QString tagName = parts[0];
        parts = parts[1].split( '=' );
        QString attributeName = parts.value( 0 );
        QString attributeValue = parts.value( 1 );

        foreach (XmlQuery e, children( tagName ))
            if (e.d->e.attribute( attributeName ) == attributeValue)
                return e;
    }
    XmlQuery xq( d->e.firstChildElement( name ), name.toUtf8().data() );
    xq.d->domdoc = this->d->domdoc;
    return xq;
}


QList<XmlQuery>
XmlQuery::children( const QString& named ) const
{
    QList<XmlQuery> elements;
    QDomNodeList nodes = d->e.elementsByTagName( named );
    for (int x = 0; x < nodes.count(); ++x) {
        XmlQuery xq( nodes.at( x ).toElement() );
        xq.d->domdoc = this->d->domdoc;
        elements += xq;
    }
    return elements;
}

XmlQuery::operator QDomElement() const
{
    return d->e;
}


XmlQuery&
XmlQuery::operator=( const XmlQuery& that )
{
    d->domdoc = that.d->domdoc;
    d->e = that.d->e;
    d->error = that.d->error;
    return *this;
}

QDebug operator<<( QDebug d, const lastfm::XmlQuery& xq )
{
    QString s;
    QTextStream t( &s, QIODevice::WriteOnly );
    QDomElement(xq).save( t, 2 );
    return d << s;
}
