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

#include "Collection.h"

#include "misc.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QVariant>
#include <QDebug>

static const int k_collectionDbVersion = 1;

// Singleton instance needs to be initialised
Collection* Collection::s_instance = NULL;


Collection::Collection()
{
    m_db = QSqlDatabase::addDatabase( "QSQLITE", "collection" );
    m_db.setDatabaseName( lastfm::dir::runtimeData().filePath( "collection.db" ) );
    
    if (!m_db.open()) {
        qDebug() << m_db.lastError();
        return;
    }

    if (!m_db.isValid()) {
        qWarning() << "collection.db connection is not valid";
        return;
    }

    if (!m_db.tables().contains( "files" ))
    {
        qDebug() << "Creating Collection database";

        query( "CREATE TABLE artists ("
                    "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "serverUid   INTEGER,"
                    "lcName      TEXT NOT NULL,"
                    "displayName TEXT NOT NULL );" );

        query( "CREATE TABLE albums ("
                    "id            INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "serverUid     INTEGER,"
                    "lcName        TEXT NOT NULL,"
                    "displayName   TEXT NOT NULL,"
                    "primaryArtist INTEGER NOT NULL );" );

        query( "CREATE UNIQUE INDEX album_names_idx ON albums ( primaryArtist, lcName );" );

        query( "CREATE TABLE tracks ("
                    "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "lcName            TEXT NOT NULL,"
                    "displayName       TEXT NOT NULL,"
                    "primaryArtist     INTEGER NOT NULL,"
                    "primaryAlbum      INTEGER );" );

        query( "CREATE UNIQUE INDEX track_names_idx ON tracks ( primaryArtist, lcName );" );

        query( "CREATE TABLE files ("
                    "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "uri               TEXT NOT NULL,"
                    "track             INTEGER NOT NULL,"
                    "bitrate           INTEGER,"
                    "samplerate        INTEGER,"
                    "duration          INTEGER,"
                    "filesize          INTEGER,"
                    "source            INTEGER,"
                    "modificationDate  INTEGER,"
                    "lastPlayDate      INTEGER,"
                    "playCounter       INTEGER,"
                    "mbId              VARCHAR( 36 ),"
                    "fpId              INTEGER );" );

        query( "CREATE UNIQUE INDEX files_uri_idx ON files ( uri );" );
        query( "CREATE INDEX files_track_idx ON files ( track );" );
        query( "CREATE INDEX files_fpId_idx ON files ( fpId );" );
        query( "CREATE INDEX files_source_idx ON files ( source );" );

        query( "CREATE TABLE sources ("
                    "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "name        TEXT UNIQUE,"
                    "available   INTEGER,"
                    "host        TEXT,"
                    "cost        INTEGER );" );

        query( "CREATE TABLE genres ("
                    "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "name        TEXT UNIQUE );" );

        query( "CREATE TABLE labels ("
                    "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "serverUid   INTEGER UNIQUE,"
                    "name        TEXT );" );
    }
    
    int const v = version();
    if ( v < k_collectionDbVersion )
    {
        qDebug() << "Upgrading Collection::db from" << v << "to" << k_collectionDbVersion;

        /**********************************************
         * README!!!!!!!                              *
         * Ensure you use v < x                       *
         * Ensure you do upgrades in ascending order! *
         **********************************************/
    
        if ( v < 1 )
        {
            // Norman discovered that he stored some fpId's wrong prior to 17th December 2007
            // So we have to wipe the fpIds for databases without the metadata table
            // we didn't store version information before that, which was a bad decision wasn't it?

            // this will trigger refingerprinting of every track
            query( "UPDATE files SET fpId = NULL;" );

            query( "CREATE TABLE metadata ("
                        "key         TEXT UNIQUE NOT NULL,"
                        "value       TEXT );" );

            query( "INSERT INTO metadata (key, value) VALUES ('version', '1');" );
        }


        // do last, update DB version number
        query( "UPDATE metadata set key='version', value='"
                    + QString::number( k_collectionDbVersion ) + "';" );
    }
}


Collection& //static
Collection::instance()
{
    static QMutex mutex;
    QMutexLocker locker( &mutex );
    
    if ( !s_instance )
    {
        s_instance = new Collection;
        qAddPostRoutine(destroy);
    }
    
    return *s_instance;
}


void //static
Collection::destroy()
{
    delete s_instance;
    QSqlDatabase::removeDatabase( "collection" );
}


int
Collection::version() const 
{   
    QSqlQuery sql( m_db );
    sql.exec( "SELECT value FROM metadata WHERE key='version';" );

    if ( sql.next() )
    {
        return sql.value( 0 ).toInt();
    }

    return 0;
}


bool
Collection::query( const QString& queryToken )
{
    QSqlQuery query( m_db );
    query.exec( queryToken );

    if ( query.lastError().isValid() )
    {
        qDebug() << "SQL query failed:" << query.lastQuery() << endl
                 << "SQL error was:"    << query.lastError().databaseText() << endl
                 << "SQL error type:"   << query.lastError().type();

        return false;
    }

    return true;
}


QString
Collection::fileURI( const QString& filePath )
{
    QString prefix( "file:/" );

#ifdef WIN32
    prefix = "file://";
#endif

    return prefix + QFileInfo( filePath ).absoluteFilePath();
}


QString
Collection::getFingerprintId( const QString& filePath )
{
    QSqlQuery query( m_db );
    query.prepare( "SELECT fpId FROM files WHERE uri = :uri" );
    query.bindValue( ":uri", fileURI( filePath ) );

    query.exec();
    if ( query.lastError().isValid() )
    {
        qDebug() << "SQL query failed:" << query.lastQuery() << endl
                 << "SQL error was:"    << query.lastError().databaseText() << endl
                 << "SQL error type:"   << query.lastError().type();
    }
    else if (query.next())
        return query.value( 0 ).toString();

    return "";
}


bool
Collection::setFingerprintId( const QString& filePath, QString fpId )
{
    bool isNumeric;
    int intFpId = fpId.toInt( &isNumeric );
    Q_ASSERT( isNumeric );

    QSqlQuery query( m_db );
    query.prepare( "REPLACE INTO files ( uri, track, fpId ) VALUES ( :uri, 0, :fpId )" );
    query.bindValue( ":uri", fileURI( filePath ) );
    query.bindValue( ":fpId", intFpId );
    query.exec();

    if ( query.lastError().isValid() )
    {
        qDebug() << "SQL query failed:" << query.lastQuery() << endl
                 << "SQL error was:"    << query.lastError().databaseText() << endl
                 << "SQL error type:"   << query.lastError().type();

        return false;
    }

    return true;
}
