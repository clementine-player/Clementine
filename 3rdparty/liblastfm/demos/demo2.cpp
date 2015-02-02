/*
   This software is in the public domain, furnished "as is", without technical 
   support, and with no warranty, express or implied, as to its usefulness for 
   any purpose.
*/

#include "ws.h"
#include "misc.h"
#include "XmlQuery.h"
#include "Artist.h"

#include <QtCore>


struct MyCoreApp : QCoreApplication
{
    Q_OBJECT
    
public:
    MyCoreApp( int& argc, char**& argv ) : QCoreApplication( argc, argv )
    {}
    
private slots:
    void onWsError( lastfm::ws::Error e )
    {
        // QNetworkReply will invoke this slot on application level errors
        // mostly this is only stuff like Ws::InvalidSessionKey and
        // Ws::InvalidApiKey    
        qWarning() << e;
    }
};


int main( int argc, char** argv )
{
    MyCoreApp app( argc, argv );
    // this is used to generate the UserAgent for webservice requests
    // please set it to something sensible in your application
    app.setApplicationName( "liblastfm" );
    
////// you'll need to fill these in for this demo to work
#warning credentials need to be filled in
    lastfm::ws::Username = "";
    lastfm::ws::ApiKey = "";
    lastfm::ws::SharedSecret = "";
    QString password = "";

////// Usually you never have to construct an Last.fm WS API call manually
    // eg. Track.getTopTags() just returns a QNetworkReply* but authentication is
    // different.
    // We're using getMobileSession here as we're a console app, but you 
    // should use getToken if you can as the user will be presented with a
    // route that feels my trustworthy to them than entering their password
    // into some random app they just downloaded... ;)
    QMap<QString, QString> params;
    params["method"] = "auth.getMobileSession";
    params["username"] = lastfm::ws::Username;
    params["authToken"] = lastfm::md5( (lastfm::ws::Username + lastfm::md5( password.toUtf8() )).toUtf8() );
    QNetworkReply* reply = lastfm::ws::post( params );
    
    // never do this when an event loop is running it's a real HACK
    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();
    
    try
    {
    ////// Usually there is a convenience function to decode the output from
        // ws calls too, but again, authentication is different. We think you
        // need to handle it yourselves :P Also conveniently it means you
        // can learn more about what our webservices return, eg. this service
        // will return an XML document like this:
        //
        // <lfm status="ok">
        //   <session>
        //     <name>mxcl</name>
        //      <key>d580d57f32848f5dcf574d1ce18d78b2</key>
        //      <subscriber>1</subscriber>
        //   </session>
        // </lfm>
        //
        // If status is not "ok" then this function throws
        lastfm::XmlQuery const lfm;
#warning this code needs to be ported to new api, executable is most likely broken
        //= lastfm::ws::parse( reply );

        // replace username; because eg. perhaps the user typed their
        // username with the wrong case
        lastfm::ws::Username = lfm["session"]["name"].text();
        
        // we now have a session key, you should save this, forever! Really.
        // DO NOT AUTHENTICATE EVERY TIME THE APP STARTS! You only have to do
        // this once. Or again if the user deletes your key on the site. If 
        // that happens you'll get notification to your onWsError() function,
        // see above.
        lastfm::ws::SessionKey = lfm["session"]["key"].text();
        
        qDebug() << "sk:" << lastfm::ws::SessionKey;
        
    ////// because the SessionKey is now set, the AuthenticatedUser class will
        // work. And we can call authenticated calls
        QNetworkReply* reply;
#warning this code needs to be ported to new api, executable is most likely broken
        //= lastfm::AuthenticatedUser().getRecommendedArtists();
        // again, you shouldn't do this.. ;)
        QEventLoop loop;
        loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
        loop.exec();
        
        // yay, a list rec'd artists to stderr :)
        qDebug() << lastfm::Artist::list( reply );
    }
    catch (std::runtime_error& e)
    {
        // lastfm::ws::parse() can throw lastfm::ws::ParseError, this 
        // exception derives std::runtime_error
        qWarning() << e.what();
        return 1;
    }
}


#include "demo2.moc"
