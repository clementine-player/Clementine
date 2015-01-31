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
#include "Tag.h"
#include "User.h"
#include "UrlBuilder.h"
#include "XmlQuery.h"
#include "ws.h"

#include <QDebug>

using lastfm::Tag;
using lastfm::User;


class lastfm::TagPrivate
{
public:
    QString name;
};


Tag::Tag( const QString& name )
    : d( new TagPrivate )
{
    d->name = name;
}


Tag::Tag( const Tag& that )
    : d( new TagPrivate( *that.d ) )
{
}


Tag::~Tag()
{
    delete d;
}

Tag::operator QString() const
{
    return d->name;
}


QString
Tag::name() const
{
    return d->name;
}


lastfm::Tag
Tag::operator=( const Tag& that ) const
{
    return Tag( that.name() );
}


bool
Tag::operator<( const Tag& that ) const
{
    return this->d->name < that.d->name;
}


QUrl
Tag::www() const
{
    return UrlBuilder( "tag" ).slash( d->name ).url();
}


QUrl
Tag::www( const User& user ) const
{
    return UrlBuilder( "user" ).slash( user.name() ).slash( "tags" ).slash( Tag::name() ).url();
}


QNetworkReply*
Tag::search() const
{
    QMap<QString, QString> map;
    map["method"] = "tag.search";
    map["tag"] = d->name;
    return ws::get(map);
}

//static
QNetworkReply* 
Tag::getTopTags()
{
    QMap<QString, QString> map;
    map["method"] = "tag.getTopTags";
    return ws::get(map);
}

QMap<int, QString> //static
Tag::list( QNetworkReply* r )
{
    QMap<int, QString> tags;

    XmlQuery lfm;

    if ( lfm.parse( r ) )
    {

        foreach ( XmlQuery xq, lfm.children("tag") )
            // we toLower always as otherwise it is ugly mixed case, as first
            // ever tag decides case, and Last.fm is case insensitive about it 
            // anyway
            tags.insertMulti( xq["count"].text().toInt(), xq["name"].text().toLower() );
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }

    return tags;
}
