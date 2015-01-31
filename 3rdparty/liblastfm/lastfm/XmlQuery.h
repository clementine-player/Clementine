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
#ifndef LASTFM_XMLQUERY_H
#define LASTFM_XMLQUERY_H

#include "ws.h"
#include <QDomElement>

namespace lastfm
{
    /** Qt's XmlQuery implementation is totally unimpressive, so this is a 
      * hack that feels like jQuery */
    class LASTFM_DLLEXPORT XmlQuery
    {
    public:
        /** we assume the bytearray is an XML document, this object will then
          * represent the documentElement of that document, eg. if this is a
          * Last.fm webservice response:
          *
          * XmlQuery xq = lastfm::ws::parse(response);
          * qDebug() << xq["artist"].text()
          *
          * Notice the lfm node is not referenced, that is because it is the
          * document-element of the XML document.
          */
        XmlQuery();
        XmlQuery( const XmlQuery& that );
        ~XmlQuery();

        /**
         * Fills in the XmlQuery response by parsing raw reply @param data from the
         * webservice.
         *
         * @return true if successfully parsed and the response does not signify an error,
         * false otherwise. When false is returned, parseError() contains the error.
         */
        bool parse( const QByteArray& data );

        /**
         * Convenience parse() overload that takes data from the @param reply and calls
         * deleteLater() on it.
         *
         * @return true if successfully parsed and the response does not signify an error,
         * false otherwise. When false is returned, parseError() contains the error.
         */
        bool parse( QNetworkReply* reply );

        ws::ParseError parseError() const;

        XmlQuery( const QDomElement& e, const char* name = "" );

        /** Selects a DIRECT child element, you can specify attributes like so:
          *
          * e["element"]["element attribute=value"].text();
          */
        XmlQuery operator[]( const QString& name ) const;
        QString text() const;
        QString attribute( const QString& name ) const;

        /** selects all children with specified name, recursively */
        QList<XmlQuery> children( const QString& named ) const;

        operator QDomElement() const;
        XmlQuery& operator=( const XmlQuery& that );

    private:
        class XmlQueryPrivate * const d;
    };
}

LASTFM_DLLEXPORT QDebug operator<<( QDebug d, const lastfm::XmlQuery& xq );

#endif
