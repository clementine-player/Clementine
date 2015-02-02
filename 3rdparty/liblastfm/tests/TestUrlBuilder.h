/*
   This software is in the public domain, furnished "as is", without technical
   support, and with no warranty, express or implied, as to its usefulness for
   any purpose.
*/

#ifndef LASTFM_TESTURLBUILDER_H
#define LASTFM_TESTURLBUILDER_H

#include <UrlBuilder.h>
#include <global.h>

#include <QtTest>
#include <QtNetwork>
#include <QEventLoop>


static inline int getResponseCode( const QUrl& url )
{
    QNetworkAccessManager nam;
    QNetworkReply* reply = nam.head( QNetworkRequest(url) );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    int const code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

    if (reply->error() != QNetworkReply::NoError)
        qDebug() << url << lastfm::qMetaEnumString<QNetworkReply>( reply->error(), "NetworkError" ) << code;

    return code;
}


class TestUrlBuilder : public QObject
{
    Q_OBJECT

private slots:
    void encode() /** @author <jono@last.fm> */
    {
        QFETCH( QString, input );
        QFETCH( QString, output );
        QCOMPARE( lastfm::UrlBuilder::encode( input ), output.toLatin1() );
    }

    void encode_data() /** @author <jono@last.fm> */
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("output");

        QTest::newRow( "ascii" ) << "Metallica" << "Metallica";
        QTest::newRow( "ascii alphanumeric" ) << "Apollo 440" << "Apollo+440";
        QTest::newRow( "ascii with symbols" ) << "some track [original version]" << "some+track+%5Boriginal+version%5D";
        QTest::newRow( "ascii with last.fm-special symbols" ) << "Survivalism [Revision #1]" << "Survivalism%2B%255BRevision%2B%25231%255D";
    }

    void no404() /** @author <max@last.fm> */
    {
        QFETCH( QString, artist );
        QFETCH( QString, track );

        QUrl url = lastfm::UrlBuilder( "music" ).slash( artist ).slash( "_" ).slash( track ).url();

        QCOMPARE( getResponseCode( url ), 200 );
    }

    void no404_data() /** @author <max@last.fm> */
    {
        QTest::addColumn<QString>("artist");
        QTest::addColumn<QString>("track");

        #define NEW_ROW( x, y ) QTest::newRow( x " - " y ) << x << y;
        NEW_ROW( "Air", "Radio #1" );
        NEW_ROW( "Pink Floyd", "Speak to Me / Breathe" );
        NEW_ROW( "Radiohead", "2 + 2 = 5" );
        NEW_ROW( "Above & Beyond", "World On Fire (Maor Levi Remix)" );
        #undef NEW_ROW
    }

    void test404() /** @author <max@last.fm> */
    {
        QCOMPARE( getResponseCode( QUrl("http://www.last.fm/404") ), 404 );
    }
};

#endif
