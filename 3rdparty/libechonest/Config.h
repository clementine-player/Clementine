/****************************************************************************************
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>                                    *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef ECHONEST_CONFIG_H
#define ECHONEST_CONFIG_H

#include "echonest_export.h"

#include <QByteArray>
#include <QUrl>

#include <exception>
#include <QNetworkReply>

class QNetworkAccessManager;

namespace Echonest{

    /// Creates the base URL for all GET and POST requests
    QUrl baseUrl();

    /// Creates the base URL for GET requests.
    QUrl baseGetQuery( const QByteArray& type, const QByteArray& method );

    enum ErrorType {
        /**
         * Echo Nest API errors
         */
        UnknownError = -1,
        NoError = 0,
        MissingAPIKey = 1,
        NotAllowed = 2,
        RateLimitExceeded = 3,
        MissingParameter = 4,
        InvalidParameter = 5,

        /// libechonest errors
        UnfinishedQuery = 6, /// An unfinished QNetworkReply* was passed to the parse function
        EmptyResult = 7,
        UnknownParseError = 8,

        /// QNetworkReply errors
        NetworkError = 9
    };

    class ECHONEST_EXPORT ParseError : public std::exception
    {
    public:
        explicit ParseError( ErrorType error );
        ParseError( ErrorType error, const QString& text );
        virtual ~ParseError() throw();

        ErrorType errorType() const throw();

        /**
         * If the ErrorType is NetworkError, this value contains the QNetworkReply
         *  error code that was returned.
         */
        void setNetworkError( QNetworkReply::NetworkError error ) throw();
        QNetworkReply::NetworkError networkError() const throw();

        virtual const char* what() const throw ();
    private:
        QByteArray createWhatData() const throw();

        ErrorType type;
        QString extraText;
        QNetworkReply::NetworkError nError;
        QByteArray whatData;
    };

    class ConfigPrivate;
    /**
     * This singleton contains miscellaneous settings used to access The Echo Nest
     */
    class ECHONEST_EXPORT Config {
    public:
        static Config* instance();

        /**
         * Set the API key to be used for all API requests. This MUST be set in order
         *  for any web service calls to work!
         */
        void setAPIKey( const QByteArray& apiKey );
        QByteArray apiKey() const;

        /**
         * Set the QNetworkAccessManager to use to make web service requests to
         *  The Echo Nest.
         *
         * This will register the given QNAM for the current thread. If you call this from
         *  the main thread and only make calls to libechonest from the main thread, you don't
         *  have to do any more work. However, if you are using multiple QNAMs in different threads,
         *  you must call this for each QNAM in each thread so that libechonest can use the proper
         *  thread-local QNAM. This function is thread-safe.
         *
         * Note that in all threads, if you do not set a QNAM, a default one is created and returned.
         *  In addition, if you set your own QNAM, you are responsible for deleting it.
         *
         * This will take over control of the object.
         */
        void setNetworkAccessManager( QNetworkAccessManager* nam );
        QNetworkAccessManager* nam() const;


    private:
        Config();
        ~Config();

        static Config* s_instance;

        ConfigPrivate* d;
    };

}

#endif
