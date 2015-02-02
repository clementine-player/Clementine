/*
   This software is in the public domain, furnished "as is", without technical 
   support, and with no warranty, express or implied, as to its usefulness for 
   any purpose.
*/

#include "Audioscrobbler.h"
#include "ws.h"
#include "Track.h"


#include <QtCore>
#include <QtNetwork>


struct MyCoreApp : QCoreApplication
{
    Q_OBJECT

public:
    MyCoreApp( int& argc, char** argv ) : QCoreApplication( argc, argv )
    {}

public slots:
    void onStatus( int status )
    {
        qDebug() << lastfm::Audioscrobbler::Status(status);
    }
};


int main( int argc, char** argv )
{
    // all 6 of these lines are REQUIRED in order to scrobble
    // this demo requires you to fill in the blanks as well...
    lastfm::ws::Username = "";
    lastfm::ws::ApiKey = "";
    lastfm::ws::SharedSecret = "";
    lastfm::ws::SessionKey = ""; // you need to auth to get this... try demo2
    QCoreApplication::setApplicationName( "liblastfm" );

    MyCoreApp app( argc, argv );
    
    lastfm::MutableTrack t;
    t.setArtist( "Max Howell" );
    t.setTitle( "I Told You Not To Trust Me With Your Daughter" );
    t.setDuration( 30 );
    t.stamp(); //sets track start time
    
    lastfm::Audioscrobbler as( "ass" );
    as.nowPlaying( t );
    // Audioscrobbler will submit whatever is in the cache when you call submit.
    // And the cache is persistent between sessions. So you should cache at the
    // scrobble point usually, not before
    as.cache( t );
    
    QTimer timer;
    timer.setSingleShot( true );
    timer.setInterval( 31*1000 );
    connect( &timer, SIGNAL(timeout()), &as, SLOT(submit()) );
    timer.start();
    
    app.connect( &as, SIGNAL(status(int)), SLOT(onStatus(int)) );
    
    return app.exec();
}


#include "demo3.moc"
