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

#include "Fingerprint.h"
#include "FingerprintableSource.h"
#include "Collection.h"
#include "Sha256.h"
#include "fplib/FingerprintExtractor.h"
#include "ws.h"
#include "Url.h"
#include <QDebug>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    #include <QUrlQuery>
#endif
#include <fstream>

#include "User.h"

using lastfm::Track;

static const uint k_bufferSize = 1024 * 8;
static const int k_minTrackDuration = 30;

class lastfm::FingerprintPrivate
{
    public:
        FingerprintPrivate( const Track& t )
                    : m_track( t )
                    , m_id( -1 )
                    , m_duration( 0 )
                    , m_complete( false )
        {
        }
        lastfm::Track m_track;
        QByteArray m_data;
        int m_id;
        int m_duration;
        bool m_complete;
};


lastfm::Fingerprint::Fingerprint( const Track& t )
                   : d( new FingerprintPrivate( t ) )
{
    QString id = Collection::instance().getFingerprintId( t.url().toLocalFile() );   
    if (id.size()) {
        bool b;
        d->m_id = id.toInt( &b );
        if (!b) d->m_id = -1;
    }
}


lastfm::Fingerprint::~Fingerprint()
{
    delete d;
}


lastfm::FingerprintId
lastfm::Fingerprint::id() const
{
    return d->m_id;
}


QByteArray
lastfm::Fingerprint::data() const
{
    return d->m_data;
}


void
lastfm::Fingerprint::generate( FingerprintableSource* ms ) throw( Error )
{
    //TODO throw if we can't get required metadata from the track object
    
//TODO    if (!QFileInfo( path ).isReadable())
//TODO        throw ReadError;

    int sampleRate, bitrate, numChannels;

    if ( !ms )
        throw ReadError;

    try
    {
        ms->init( d->m_track.url().toLocalFile() );
        ms->getInfo( d->m_duration, sampleRate, bitrate, numChannels );
    }
    catch (std::exception& e)
    {
        qWarning() << e.what();
        throw HeadersError;
    }
    

    if (d->m_duration < k_minTrackDuration)
        throw TrackTooShortError;
    
    ms->skipSilence();
    
    bool fpDone = false;
    fingerprint::FingerprintExtractor* extractor;
    try
    {
        extractor = new fingerprint::FingerprintExtractor;
        
        if (d->m_complete)
        {
            extractor->initForFullSubmit( sampleRate, numChannels );
        }
        else 
        {
            extractor->initForQuery( sampleRate, numChannels, d->m_duration );
            
            // Skippety skip for as long as the skipper sez (optimisation)
            ms->skip( extractor->getToSkipMs() );
            float secsToSkip = extractor->getToSkipMs() / 1000.0f;
            fpDone = extractor->process( 0,
                                         (size_t) sampleRate * numChannels * secsToSkip,
                                         false );
        }
    }
    catch (std::exception& e)
    {
        qWarning() << e.what();
        delete extractor;
        throw DecodeError;
    }
    
    const size_t PCMBufSize = 131072; 
    short* pPCMBuffer = new short[PCMBufSize];
    
    while (!fpDone)
    {
        size_t readData = ms->updateBuffer( pPCMBuffer, PCMBufSize );
        if (readData == 0)
            break;
        
        try
        {
            fpDone = extractor->process( pPCMBuffer, readData, ms->eof() );
        }
        catch ( const std::exception& e )
        {
            qWarning() << e.what();
            delete[] pPCMBuffer;
            delete extractor;
            throw InternalError;
        }
    }
    
    delete[] pPCMBuffer;
    
    if (!fpDone)
    {
        delete extractor;
        throw InternalError;
    }

    // We succeeded
    std::pair<const char*, size_t> fpData = extractor->getFingerprint();
    
    if (fpData.first == NULL || fpData.second == 0)
        {
        delete extractor;
        throw InternalError;
        }

    // Make a deep copy before extractor gets deleted
    d->m_data = QByteArray( fpData.first, fpData.second );
    delete extractor;
}


static QString sha256( const QString& path )
{
    // no clue why this is static, there was no comment when I refactored it
    // initially --mxcl
    static uint8_t pBuffer[SHA_BUFFER_SIZE+7];
    
    unsigned char hash[SHA256_HASH_SIZE];

    {
        QByteArray path8 = QFile::encodeName( path );
        std::ifstream inFile( path8.data(), std::ios::binary);
        
        SHA256Context sha256;
        SHA256Init( &sha256 );
        
        uint8_t* pMovableBuffer = pBuffer;
        
        // Ensure it is on a 64-bit boundary. 
        INTPTR offs;
        if ((offs = reinterpret_cast<INTPTR>(pBuffer) & 7L))
            pMovableBuffer += 8 - offs;
        
        unsigned int len;
        
        for (;;)
        {
            inFile.read( reinterpret_cast<char*>(pMovableBuffer), SHA_BUFFER_SIZE );
            len = inFile.gcount();
            
            if (len == 0)
                break;
            
            SHA256Update( &sha256, pMovableBuffer, len );
        }
        
        SHA256Final( &sha256, hash );
    }

    QString sha;
    for (int i = 0; i < SHA256_HASH_SIZE; ++i) 
    {
        QString hex = QString("%1").arg(uchar(hash[i]), 2, 16,
                                        QChar('0'));
        sha.append(hex);
    }

    return sha;
}


static QByteArray number( uint n )
{
    return n ? QByteArray::number( n ) : "";
}


QNetworkReply*
lastfm::Fingerprint::submit() const
{    
    if (d->m_data.isEmpty())
        return 0;
    
    //Parameters understood by the server according to the MIR team: 
    //{ "trackid", "recordingid", "artist", "album", "track", "duration", 
    //  "tracknum", "username", "sha256", "ip", "fpversion", "mbid", 
    //  "filename", "genre", "year", "samplerate", "noupdate", "fulldump" }
    
    Track const t = d->m_track;
    QString const path = t.url().toLocalFile();
    QFileInfo const fi( path );

    lastfm::Url url( QUrl( "http://ws.audioscrobbler.com/fingerprint/query/" ) );
    url.addQueryItem( "username", lastfm::User().name() );
    url.addQueryItem( "artist", t.artist() );
    url.addQueryItem( "album", t.album() );
    url.addQueryItem( "track", t.title() );
    url.addQueryItem( "duration", number( d->m_duration > 0 ? d->m_duration : t.duration() ) );
    url.addQueryItem( "mbid", t.mbid() );
    url.addQueryItem( "filename", fi.completeBaseName() );
    url.addQueryItem( "fileextension", fi.completeSuffix() );
    url.addQueryItem( "tracknum", number( t.trackNumber() ) );
    url.addQueryItem( "sha256", sha256( path ) );
    url.addQueryItem( "time", number(QDateTime::currentDateTime().toTime_t()) );
    url.addQueryItem( "fpversion", QByteArray::number((int)fingerprint::FingerprintExtractor::getVersion()) );
    url.addQueryItem( "fulldump", d->m_complete ? "true" : "false" );
    url.addQueryItem( "noupdate", "false" );

    //FIXME: talk to mir about submitting fplibversion

    QNetworkRequest request( url.url() );
    request.setHeader( QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=----------------------------8e61d618ca16" );

    QByteArray bytes;
    bytes += "------------------------------8e61d618ca16\r\n";
    bytes += "Content-Disposition: ";
    bytes += "form-data; name=\"fpdata\"";
    bytes += "\r\n\r\n";
    bytes += d->m_data;
    bytes += "\r\n";
    bytes += "------------------------------8e61d618ca16--\r\n";

    qDebug() << url.url();
    qDebug() << "Fingerprint size:" << bytes.size() << "bytes";

    return lastfm::nam()->post( request, bytes );
}


void
lastfm::Fingerprint::decode( QNetworkReply* reply, bool* complete_fingerprint_requested ) throw( Error )
{
    // The response data will consist of a number and a string.
    // The number is the fpid and the string is either FOUND or NEW
    // (or NOT FOUND when noupdate was used). NEW means we should
    // schedule a full fingerprint.
    //
    // In the case of an error, there will be no initial number, just
    // an error string.
    
    reply->deleteLater();
    QString const response( reply->readAll() );
    QStringList const list = response.split( ' ' );

    QString const fpid = list.value( 0 );
    QString const status = list.value( 1 );
       
    if (response.isEmpty() || list.count() < 2 || response == "No response to client error")
        goto bad_response;
    if (list.count() != 2)
        qWarning() << "Response looks bad but continuing anyway:" << response;

    {
        // so variables go out of scope before jump to label
        // otherwise compiler error on GCC 4.2
        bool b;
        uint fpid_as_uint = fpid.toUInt( &b );
        if (!b) goto bad_response;
    
        Collection::instance().setFingerprintId( d->m_track.url().toLocalFile(), fpid );
    
        if (complete_fingerprint_requested)
            *complete_fingerprint_requested = (status == "NEW");

        d->m_id = (int)fpid_as_uint;
        return;
    }

bad_response:
    qWarning() << "Response is bad:" << response;
    throw BadResponseError;
}


lastfm::CompleteFingerprint::CompleteFingerprint( const lastfm::Track& t ) : Fingerprint( t )
{
    d->m_complete = true;
}


lastfm::CompleteFingerprint::~CompleteFingerprint()
{
}


QDebug operator<<( QDebug d, lastfm::Fingerprint::Error e )
{
    #define CASE(x) case lastfm::Fingerprint::x: return d << #x;
    switch (e)
    {
        CASE(ReadError)
        CASE(HeadersError)
        CASE(DecodeError)
        CASE(TrackTooShortError)
        CASE(BadResponseError)
        CASE(InternalError)
    }
    #undef CASE

    return d;
}
