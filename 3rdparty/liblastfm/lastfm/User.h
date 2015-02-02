/*
   Copyright 2009-2010 Last.fm Ltd.
      - Primarily authored by Max Howell, Jono Cole, Doug Mansell and Michael Coffey

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
#ifndef LASTFM_USER_H
#define LASTFM_USER_H

#include <QStringList>

#include "AbstractType.h"
#include "ws.h"

namespace lastfm
{
    class UserList;
    class XmlQuery;
    class Artist;

    class LASTFM_DLLEXPORT Gender
    {

    public:
        Gender();
        Gender( const Gender& gender );
        Gender( const QString& ss );
        ~Gender();

        bool known() const;
        bool male() const;
        bool female() const;

        QString toString() const;
        lastfm::Gender& operator=( const lastfm::Gender& that );

    private:
        class GenderPrivate;
        GenderPrivate * const d;
    };

    class LASTFM_DLLEXPORT User : public AbstractType
    {
    public:
        enum Type
        {
            TypeUser,
            TypeSubscriber,
            TypeModerator,
            TypeStaff,
            TypeAlumni
        };

    public:
        User();
        User( const QString& name );
        User( const XmlQuery& xml );
        User( const User& user );
        ~User();

        lastfm::User& operator=( const lastfm::User& that );
        bool operator==(const lastfm::User& that) const;
        bool operator<(const lastfm::User& that) const;

        operator QString() const;

        QString name() const;
        void setName( const QString& name );

        Type type() const;
        void setType( Type type );

        bool isSubscriber() const;
        void setIsSubscriber( bool subscriber );

        bool canBootstrap() const;
        void setCanBootstrap( bool canBootstrap );

        quint32 scrobbleCount() const;
        void setScrobbleCount( quint32 scrobblesCount );

        QDateTime dateRegistered() const;
        void setDateRegistered( const QDateTime& date );

        Gender gender() const;
        QString country() const;

        QString realName() const;
        void setRealName( const QString& realName );

        QUrl imageUrl( ImageSize size = LargeImage, bool square = false ) const;
        void setImages( const QList<QUrl>& images );

        unsigned short age() const;
        void setAge( unsigned short age );

        void setGender( const QString& s );
        void setCountry( const QString& country );
    
        /** use Tag::list() on the response to get a WeightedStringList */
        QNetworkReply* getTopTags() const;

        /** use User::list() on the response to get a UserList */
        QNetworkReply* getFriends(  bool recentTracks = false, int limit = 50, int page = 1 ) const;
        QNetworkReply* getFriendsListeningNow( int limit = 50, int page = 1 ) const;
        QNetworkReply* getFriendsThatListenTo( const lastfm::Artist& artist, int limit = 50, int page = 1 ) const;
        QNetworkReply* getNeighbours( int limit = 50, int page = 1 ) const;
    
        QNetworkReply* getLovedTracks( int limit = 50, int page = 1 ) const;
        QNetworkReply* getPlaylists() const;
        QNetworkReply* getTopArtists( QString period = "overall", int limit = 50, int page = 1 ) const;
        QNetworkReply* getRecentTracks( int limit = 50, int page = 1 ) const;
        QNetworkReply* getRecentArtists() const;
        QNetworkReply* getRecentStations(  int limit = 10, int page = 1  ) const;
        QNetworkReply* getRecommendedArtists( int limit = 50, int page = 1 ) const;

        /** you can only get information about the any user */
        static QNetworkReply* getInfo( const QString& username = lastfm::ws::Username );

        /** a verbose string, eg. "A man with 36,153 scrobbles" */
        QString getInfoString() const;
    
        static UserList list( QNetworkReply* );

        QString toString() const;
        QDomElement toDomElement( QDomDocument& ) const;
    
        /** the user's profile page at www.last.fm */
        QUrl www() const;
    
        /** Returns the match between the logged in user and the user which this
          * object represents (if < 0.0f then not set) */
        float match() const;
    
    protected:        
        QMap<QString, QString> params( const QString& method ) const;

    protected:
        class UserPrivate;
        UserPrivate * const d;
    };

    class LASTFM_DLLEXPORT UserList
    {
    public:
        UserList();
        UserList( const XmlQuery& query );
        UserList( const UserList& other );
        ~UserList();
        UserList& operator=( const UserList& other );

        int totalUsers();
        int totalPages();
        int currentPage();
        int usersPerPage();
        QList<User> users();

    private:
        class UserListPrivate * const d;
    };
}

#endif
