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

/** Class that we use to store fingerprints, basically
  */

#ifndef COLLECTION_H
#define COLLECTION_H

#include <QObject>
#include <QSqlDatabase>


/** @author: <chris@last.fm> */
class Collection
{
public:
    static Collection& instance();

    /** \brief Temp method: Gets a fingerprint id. Returns "" if none found. */
    QString getFingerprintId( const QString& filePath );

    /** \brief Temp method: Sets a fingerprint id. */
    bool setFingerprintId( const QString& filePath, QString fpId );

private:
    Collection();

    /** the database version
        * version 0: up until 1.4.1
        * version 1: from 1.4.2 */
    int version() const;
    bool query( const QString& queryToken );
    QString fileURI( const QString& filePath );

    static void destroy();    
    
    static Collection* s_instance;
    QSqlDatabase m_db;
};

#endif // COLLECTION_H
