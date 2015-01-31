/*
   Copyright 2009 Last.fm Ltd. 
   Copyright 2009 John Stamp <jstamp@users.sourceforge.net>
   Portions Copyright 2003-2005 M. Bakker, Nero AG, http://www.nero.com
      - Adapted from main.c found in the FAAD2 source tarball.

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
#include "AacSource.h"
#include "AacSource_p.h"

#include <QFile>
#include <algorithm>
#include <cassert>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <errno.h>
#include <string.h>


////////////////////////////////////////////////////////////////////////
//
// AAC_File
//
////////////////////////////////////////////////////////////////////////
AAC_File::AAC_File(const QString& fileName, int headerType)
    : m_fileName(fileName)
    , m_inBuf(NULL)
    , m_inBufSize(0)
    , m_decoder(0)
    , m_overflow(static_cast<unsigned char*>(malloc( sizeof(unsigned char) * 1024 )))
    , m_overflowSize(0)
    , m_header(headerType)
{
}


AAC_File::~AAC_File()
{
    // common
    if ( m_decoder )
    {
        NeAACDecClose( m_decoder );
        m_decoder = NULL;
    }
    if ( m_inBuf )
    {
        free( m_inBuf );
        m_inBufSize = 0;
        m_inBuf = NULL;
    }
    if ( m_overflow )
    {
        free( m_overflow );
        m_overflowSize = 0;
        m_overflow = NULL;
    }
}



////////////////////////////////////////////////////////////////////////
//
// AAC with ADTS or ADIF headers
//
////////////////////////////////////////////////////////////////////////


#define MAX_CHANNELS 6 // Output will get mixed down to 2 channels
#define ADTS_HEADER_SIZE 8

static int adts_sample_rates[] =
{
    96000,
    88200,
    64000,
    48000,
    44100,
    32000,
    24000,
    22050,
    16000,
    12000,
    11025,
     8000,
     7350,
        0,
        0,
        0
};

AAC_ADTS_File::AAC_ADTS_File( const QString& fileName, int headerType ) : AAC_File(fileName, headerType)
    , m_file( NULL )
    , m_adifSamplerate( 0 )
    , m_adifChannels( 0 )
{
}


AAC_ADTS_File::~AAC_ADTS_File()
{
    if ( m_file )
    {
        fclose( m_file );
    }
}


void AAC_ADTS_File::fillBuffer( FILE*& fp, unsigned char*& buf, size_t& bufSize, const size_t bytesConsumed )
{
    size_t bread;

    if ( bytesConsumed > 0 )
    {
        if ( bufSize )
            memmove( (void*)buf, (void*)(buf + bytesConsumed), bufSize*sizeof(unsigned char) );

        bread = fread( (void*)(buf + bufSize), 1, bytesConsumed, fp );
        bufSize += bread;

        if ( bufSize > 3 )
        {
            if ( memcmp( buf, "TAG", 3 ) == 0 )
                bufSize = 0;
        }
        if ( bufSize > 11 )
        {
            if ( memcmp( buf, "LYRICSBEGIN", 11 ) == 0 )
                bufSize = 0;
        }
        if ( bufSize > 8 )
        {
            if ( memcmp( buf, "APETAGEX", 8 ) == 0 )
                bufSize = 0;
        }
    }
}


void AAC_ADTS_File::parse( FILE*& fp, unsigned char*& buf, size_t& bufSize, int &bitrate, double &length )
{
    unsigned int frames, frame_length = 0;
    int t_framelength = 0;
    int samplerate = 0;
    double frames_per_sec, bytes_per_frame;

    // Read all frames to ensure correct time and bitrate
    for ( frames = 0; /* */; frames++ )
    {
        fillBuffer( fp, buf, bufSize, frame_length );

        if ( bufSize > 7 )
        {
            /* check syncword */
            if ( !( (buf[0] == 0xFF) && ((buf[1] & 0xF6) == 0xF0) ) )
                break;

            if ( frames == 0 )
                samplerate = adts_sample_rates[ (buf[2] & 0x3c) >> 2 ];

            frame_length = (  ((buf[3] & 0x3) << 11)
                            | ((buf[4]) << 3)
                            | (buf[5] >> 5) );

            t_framelength += frame_length - ADTS_HEADER_SIZE;

            if ( frame_length > bufSize )
                break;

            bufSize -= frame_length;
        }
        else
        {
            break;
        }
    }

    frames_per_sec = samplerate / 1024.0;

    if ( frames != 0 )
        bytes_per_frame = t_framelength / frames;
    else
        bytes_per_frame = 0;

    bitrate = static_cast<int>(8 * bytes_per_frame * frames_per_sec + 0.5);

    if ( frames_per_sec != 0 )
        length = frames / frames_per_sec;
    else
        length = 1;
}


int32_t AAC_ADTS_File::commonSetup( FILE*& fp, NeAACDecHandle& decoder, unsigned char*& buf, size_t& bufSize, uint32_t& samplerate, uint8_t& channels )
{
    samplerate = 0;
    channels = 0;

    fp = fopen(QFile::encodeName(m_fileName), "rb" );
    if( !fp )
    {
        std::cerr << "ERROR: Failed to open " << strerror( errno ) << std::endl;
        return -1;
    }

    if ( !(buf = static_cast<unsigned char*>( malloc(FAAD_MIN_STREAMSIZE*MAX_CHANNELS)) ) )
    {
        std::cerr << "Memory allocation error" << std::endl;
        fclose ( fp );
        return -1;
    }

    memset( buf, 0, FAAD_MIN_STREAMSIZE*MAX_CHANNELS );

    bufSize = fread( buf, 1, FAAD_MIN_STREAMSIZE * MAX_CHANNELS, fp );

    int tagsize = 0;
    if ( !memcmp( buf, "ID3", 3 ) )
    {
        /* high bit is not used */
        tagsize = (buf[6] << 21) | (buf[7] << 14) |
            (buf[8] <<  7) | (buf[9] <<  0);

        tagsize += 10;
        bufSize -= tagsize;
        fillBuffer( fp, buf, bufSize, tagsize );
    }

    decoder = NeAACDecOpen();

    /* Set configuration */
    NeAACDecConfigurationPtr config;
    config = NeAACDecGetCurrentConfiguration(decoder);
    config->outputFormat = FAAD_FMT_16BIT;
    config->downMatrix = 1; // Turn 5.1 channels into 2
    NeAACDecSetConfiguration( decoder, config);

    int32_t initval = 0;
    if ((initval = NeAACDecInit(decoder, buf,
        FAAD_MIN_STREAMSIZE*MAX_CHANNELS, &samplerate, &channels)) < 0)
    {
        std::cerr << "Error: could not set up AAC decoder" << std::endl;
        if ( buf )
            free( buf );
        buf = NULL;
        NeAACDecClose( decoder );
        decoder = NULL;
        fclose( fp );
        fp = NULL;
    }
    return initval;
}


bool AAC_ADTS_File::init()
{
    uint32_t initSamplerate = 0;
    uint8_t initChannels = 0;
    int32_t initval = commonSetup( m_file, m_decoder, m_inBuf, m_inBufSize, initSamplerate, initChannels );

     if ( initval >= 0 )
     {
        m_inBufSize -= initval;
        fillBuffer( m_file, m_inBuf, m_inBufSize, initval );

        // These two only needed for skipping AAC ADIF files
        m_adifSamplerate = initSamplerate;
        m_adifChannels = initChannels;

        return true;
     }

     throw std::runtime_error( "ERROR: Could not initialize AAC file reader!" );
     return false;
}


/*QString AAC_ADTS_File::getMbid()
{
    char out[MBID_BUFFER_SIZE];
    int const r = getMP3_MBID(QFile::encodeName(m_fileName), out);
    if ( r == 0 )
        return QString::fromLatin1( out );
    return QString();
}*/

void AAC_ADTS_File::getInfo( int& lengthSecs, int& samplerate, int& bitrate, int& nchannels )
{
    long fileread;
    uint32_t initSamplerate;
    uint8_t initChannels;
    double initLength = 0;
    unsigned char *tempBuf = NULL;
    size_t tempBufSize;
    FILE *fp = NULL;
    NeAACDecHandle decoder = NULL;
    commonSetup( fp, decoder, tempBuf, tempBufSize, initSamplerate, initChannels );

    long origpos = ftell( fp );
    fseek( fp, 0, SEEK_END );
    fileread = ftell( fp );
    fseek( fp, origpos, SEEK_SET );

    if ( (tempBuf[0] == 0xFF) && ((tempBuf[1] & 0xF6) == 0xF0) )
    {
        parse( fp, tempBuf, tempBufSize, bitrate, initLength );
    }
    else if (memcmp(tempBuf, "ADIF", 4) == 0)
    {
        int skip_size = (tempBuf[4] & 0x80) ? 9 : 0;
        bitrate = ((tempBuf[4 + skip_size] & 0x0F)<<19) |
            (tempBuf[5 + skip_size]<<11) |
            (tempBuf[6 + skip_size]<<3) |
            (tempBuf[7 + skip_size] & 0xE0);

        if ( fileread != 0)
        {
            initLength = static_cast<double>(fileread) * 8 / bitrate + 0.5;
        }
    }

    lengthSecs = static_cast<int>(initLength);
    nchannels = initChannels;
    samplerate = initSamplerate;

    if ( decoder )
        NeAACDecClose( decoder );
    if ( fp )
        fclose( fp );
    if ( tempBuf )
        free( tempBuf );
}


void AAC_ADTS_File::skip( const int mSecs )
{
    if ( m_header == AAC_ADTS )
    {
        // As AAC is VBR we need to check all ADTS headers to enable seeking...
        // There is no other solution
        unsigned char header[8];
        unsigned int frameCount, frameLength;
        double seconds = 0;

        // We need to find the ATDS syncword so rewind to the beginning
        // of the unprocessed data.
        if ( m_inBufSize > 0 )
        {
            fseek ( m_file, -m_inBufSize, SEEK_CUR );
            m_inBufSize = 0;
        }

        for( frameCount = 1; seconds * 1000 < mSecs; frameCount++ )
        {
            if ( fread( header, 1, ADTS_HEADER_SIZE, m_file ) != ADTS_HEADER_SIZE )
            {
                break;
            }
            if ( !strncmp( (char*)header, "ID3", 3 ) )
            {
                // high bit is not used
                unsigned char rest[2];
                fread( rest, 1, 2, m_file );
                int tagsize = (header[6] << 21) | (header[7] << 14) |
                    (rest[0] <<  7) | (rest[1] <<  0);

                fseek( m_file, tagsize, SEEK_CUR );
                fread( header, 1, ADTS_HEADER_SIZE, m_file );
            }
            if ( !((header[0] == 0xFF) && ((header[1] & 0xF6) == 0xF0)) )
            {
                std::cerr << "Error: Bad frame header; file may be corrupt!" << std::endl;
                break;
            }

            int samplerate = adts_sample_rates[ (header[2] & 0x3c) >> 2 ];
            frameLength = ( ( header[3] & 0x3 ) << 11 )
                          | ( header[4] << 3 )
                          | ( header[5] >> 5 );

            if ( samplerate > 0 )
                seconds += 1024.0 / samplerate;
            else
            {
                std::cerr << "Error: Bad frame header; file may be corrupt!" << std::endl;
                break;
            }

            if ( fseek( m_file, frameLength - ADTS_HEADER_SIZE, SEEK_CUR ) == -1 )
                break;
        }
        m_inBufSize = fread( m_inBuf, 1, FAAD_MIN_STREAMSIZE * MAX_CHANNELS, m_file );
    }
    else if ( m_header == AAC_ADIF )
    {
        // AAC ADIF is even worse.  There's only the one header at the
        // beginning of the file.  If you want to skip forward, you have to
        // decode block by block and check how far along you are.  Lovely, eh?

        unsigned long totalSamples = 0;
        void *sampleBuffer = NULL;

        do
        {
            NeAACDecFrameInfo frameInfo;
            sampleBuffer = NeAACDecDecode(m_decoder, &frameInfo, m_inBuf, static_cast<uint32_t>(m_inBufSize) );
            totalSamples += frameInfo.samples;
            if ( frameInfo.bytesconsumed > 0 )
            {
                m_inBufSize -= frameInfo.bytesconsumed;
                fillBuffer( m_file, m_inBuf, m_inBufSize, frameInfo.bytesconsumed );
            }
            if ( totalSamples >= ( mSecs * m_adifSamplerate * m_adifChannels / 1000 ) )
                break;
        } while ( sampleBuffer != NULL );
    }
}


void AAC_ADTS_File::postDecode(unsigned long bytesConsumed)
{
    m_inBufSize -= bytesConsumed;
    fillBuffer( m_file, m_inBuf, m_inBufSize, bytesConsumed );
}


////////////////////////////////////////////////////////////////////////
//
// AAC in an MP4 wrapper
//
////////////////////////////////////////////////////////////////////////


uint32_t read_callback( void *user_data, void *buffer, uint32_t length )
{
    return static_cast<uint32_t>(fread( buffer, 1, length, static_cast<FILE*>(user_data) ));
}


uint32_t seek_callback( void *user_data, uint64_t position )
{
    return fseek( static_cast<FILE*>(user_data), static_cast<long>(position), SEEK_SET );
}

AAC_MP4_File::AAC_MP4_File( const QString& fileName, int headerType ) : AAC_File(fileName, headerType)
    , m_mp4AudioTrack( -1 )
    , m_mp4SampleId( 0 )
    , m_mp4File ( NULL )
    , m_mp4cb ( NULL )
{
}

int32_t AAC_MP4_File::readSample()
{
    unsigned int bsize;
    int32_t rc = mp4ff_read_sample( m_mp4File, m_mp4AudioTrack, m_mp4SampleId, &m_inBuf,  &bsize );
    m_inBufSize = bsize;
    // Not necessarily an error.  Could just mean end of file.
    //if ( rc == 0 )
    //    std::cerr << "Reading samples failed." << std::endl;
    return rc;
}


int32_t AAC_MP4_File::getTrack( const mp4ff_t *f )
{
    // find AAC track
    int32_t numTracks = mp4ff_total_tracks( f );

    for ( int32_t i = 0; i < numTracks; i++ )
    {
        unsigned char *buff = NULL;
        unsigned int buff_size = 0;
        mp4AudioSpecificConfig mp4ASC;

        mp4ff_get_decoder_config( f, i, &buff, &buff_size );

        if ( buff )
        {
            int8_t rc = NeAACDecAudioSpecificConfig( buff, buff_size, &mp4ASC );
            free( buff );

            if ( rc < 0 )
                continue;
            return i;
        }
    }

    // can't decode this, probably DRM
    return -1;
}


bool AAC_MP4_File::commonSetup( NeAACDecHandle& decoder, mp4ff_callback_t*& cb, FILE*& fp, mp4ff_t*& mp4, int32_t& audioTrack )
{
    fp = fopen(QFile::encodeName(m_fileName), "rb");
    if ( !fp )
    {
        throw std::runtime_error( "Error: failed to open AAC file!" );
        return false;
    }

    decoder = NeAACDecOpen();

    // Set configuration
    NeAACDecConfigurationPtr config;
    config = NeAACDecGetCurrentConfiguration( decoder );
    config->outputFormat = FAAD_FMT_16BIT;
    config->downMatrix = 1; // Turn 5.1 channels into 2
    NeAACDecSetConfiguration( decoder, config );

    // initialise the callback structure
    cb = static_cast<mp4ff_callback_t*>( malloc( sizeof(mp4ff_callback_t) ) );

    cb->read = read_callback;
    cb->seek = seek_callback;
    cb->user_data = fp;

    mp4 = mp4ff_open_read( cb );

    if ( !mp4 )
    {
        // unable to open file
        free( cb );
        cb = NULL;
        NeAACDecClose( decoder );
        decoder = NULL;
        fclose( fp );
        fp = NULL;
        throw std::runtime_error( "Error: failed to set up AAC decoder!" );
        return false;
    }

    if ( ( audioTrack = getTrack( mp4 )) < 0 )
    {
        free( cb );
        cb = NULL;
        NeAACDecClose( decoder );
        decoder = NULL;
        fclose( fp );
        fp = NULL;
        mp4ff_close( mp4 );
        mp4 = NULL;
        audioTrack = 0;
        throw std::runtime_error( "Error: Unable to find an audio track. Is the file DRM protected?" );
        return false;
    }
    return true;
}


/*QString AAC_MP4_File::getMbid()
{
    int j = mp4ff_meta_get_num_items( m_mp4File );
    if ( j > 0 )
    {
        int k;
        for ( k = 0; k < j; k++ )
        {
            char *tag = NULL, *item = NULL;
            if ( mp4ff_meta_get_by_index( m_mp4File, k, &item, &tag ) )
            {
                if ( item != NULL && tag != NULL )
                {
                    QString key(item);
                    if ( key.toLower() == "musicbrainz track id" )
                    {
                        QString ret(tag);
                        free( item );
                        free( tag );
                        return ret;
                    }
                    free( item );
                    free( tag );
                }
            }
        }
    }
    return QString();
}*/


void AAC_MP4_File::getInfo( int& lengthSecs, int& samplerate, int& bitrate, int& nchannels )
{
    FILE* fp = NULL;
    mp4ff_callback_t *cb = NULL;
    NeAACDecHandle decoder = NULL;
    mp4ff_t* mp4 = NULL;
    int32_t audioTrack;

    bool success = commonSetup( decoder, cb, fp, mp4, audioTrack );

    if ( success )
    {
        // get basic file info
        mp4AudioSpecificConfig mp4ASC;
        unsigned char* buffer = NULL;
        unsigned int buffer_size = 0;
        double f = 1024.0;
        unsigned int framesize = 1024;

        int32_t samples = mp4ff_num_samples( mp4, audioTrack );

        if ( buffer )
        {
            if ( NeAACDecAudioSpecificConfig(buffer, buffer_size, &mp4ASC) >= 0 )
            {
                if ( mp4ASC.frameLengthFlag == 1 )
                    framesize = 960;
                if ( mp4ASC.sbr_present_flag == 1 )
                    framesize *= 2;
                if ( mp4ASC.sbr_present_flag == 1 )
                    f = f * 2.0;
            }
            free( buffer );
        }

        samplerate = mp4ff_get_sample_rate( mp4, audioTrack );
        if ( samplerate > 0 )
            lengthSecs = static_cast<int>(samples * f / samplerate + 0.5);
        bitrate = mp4ff_get_avg_bitrate( mp4, audioTrack );
        nchannels = mp4ff_get_channel_count( mp4, audioTrack );

        mp4ff_close( mp4 );
        NeAACDecClose( decoder );
        free( cb );
        fclose( fp );
    }
}


bool AAC_MP4_File::init()
{
    FILE* fp = NULL;

    bool success = commonSetup( m_decoder, m_mp4cb, fp, m_mp4File, m_mp4AudioTrack );
    if ( !success )
        return false;

    unsigned char* buffer = NULL;
    unsigned int buffer_size = 0;
    uint32_t samplerate;
    uint8_t channels;

    mp4ff_get_decoder_config( m_mp4File, m_mp4AudioTrack, &buffer, &buffer_size );

    if( NeAACDecInit2( m_decoder, buffer, buffer_size, &samplerate, &channels) < 0 )
    {
        // If some error initializing occured, skip the file
        if ( fp )
            fclose( fp );
        throw std::runtime_error( "Error: unable to initialize AAC decoder library!" );
        return false;
    }

    if ( buffer )
        free( buffer );

    return true;
}


void AAC_MP4_File::postDecode(unsigned long)
{
            free( m_inBuf );
            m_inBuf = NULL;
            m_mp4SampleId++;
}

void AAC_MP4_File::skip( const int mSecs )
{
    double dur = 0.0;
    int f = 1;
    unsigned char *buff = NULL;
    unsigned int buff_size = 0;
    uint32_t totalSamples = mp4ff_num_samples( m_mp4File, m_mp4AudioTrack );
    mp4AudioSpecificConfig mp4ASC;

    mp4ff_get_decoder_config( m_mp4File, m_mp4AudioTrack, &buff, &buff_size );

    if ( buff )
    {
        int8_t rc = NeAACDecAudioSpecificConfig( buff, buff_size, &mp4ASC );
        free( buff );
        if ( rc >= 0 && mp4ASC.sbr_present_flag == 1 )
            f = 2;

        // I think the f multiplier is needed here.
        while ( dur * 1000.0 * f / static_cast<double>(mp4ASC.samplingFrequency) < mSecs && m_mp4SampleId < totalSamples )
        {
            dur += mp4ff_get_sample_duration( m_mp4File, m_mp4AudioTrack, m_mp4SampleId );
            m_mp4SampleId++;
        }
    }
    else
        std::cerr << "Error: could not skip " << mSecs << " milliseconds" << std::endl;
}


AAC_MP4_File::~AAC_MP4_File()
{
    if ( m_mp4File )
        mp4ff_close( m_mp4File );
    if ( m_mp4cb )
    {
        free( m_mp4cb );
    }
}


////////////////////////////////////////////////////////////////////////
//
// AacSource
//
////////////////////////////////////////////////////////////////////////

AacSource::AacSource()
    : m_eof( false )
    , m_aacFile( NULL )
{}


AacSource::~AacSource()
{
    delete m_aacFile;
}


int AacSource::checkHeader()
{
    FILE *fp = NULL;
    unsigned char header[10];

    // check for mp4 file
    fp = fopen(QFile::encodeName(m_fileName), "rb");
    if ( !fp )
    {
        std::cerr << "Error: failed to open " << strerror( errno ) << std::endl;
        return AAC_File::AAC_UNKNOWN;
    }

    fread( header, 1, 10, fp );

    // MP4 headers
    if ( !memcmp( &header[4], "ftyp", 4 ) )
    {
        fclose( fp );
        return AAC_File::AAC_MP4;
    }

    // Skip id3 tags
    int tagsize = 0;
    if ( !memcmp( header, "ID3", 3 ) )
    {
        /* high bit is not used */
        tagsize = (header[6] << 21) | (header[7] << 14) |
            (header[8] <<  7) | (header[9] <<  0);

        tagsize += 10;
        fseek( fp, tagsize, SEEK_SET );
        fread( header, 1, 10, fp );
    }

    // Check for ADTS OR ADIF headers
    if ( (header[0] == 0xFF) && ((header[1] & 0xF6) == 0xF0) )
    {
        fclose( fp );
        return AAC_File::AAC_ADTS;
    }
    else if (memcmp(header, "ADIF", 4) == 0)
    {
        fclose( fp );
        return AAC_File::AAC_ADIF;
    }

    fclose( fp );
    return AAC_File::AAC_UNKNOWN;
}


void AacSource::getInfo( int& lengthSecs, int& samplerate, int& bitrate, int& nchannels )
{
    // get the header plus some other stuff..

    m_aacFile->getInfo( lengthSecs, samplerate, bitrate, nchannels );
}


void AacSource::init(const QString& fileName)
{
    m_fileName = fileName;
    
    int headerType = checkHeader();
    if ( headerType != AAC_File::AAC_UNKNOWN )
    {
        if ( headerType == AAC_File::AAC_MP4 )
            m_aacFile = new AAC_MP4_File(m_fileName, headerType);
        else
            m_aacFile = new AAC_ADTS_File( m_fileName, headerType );
    }    
    
    if ( m_aacFile )
        m_aacFile->init();
    else
        throw std::runtime_error( "ERROR: No suitable AAC decoder found!" );
}


/*QString AacSource::getMbid()
{
    QString mbid = m_aacFile->getMbid();
    return mbid;
}*/


void AacSource::skip( const int mSecs )
{
    if ( mSecs < 0 || !m_aacFile->m_decoder )
        return;

        m_aacFile->skip( mSecs );
}


void AacSource::skipSilence(double silenceThreshold /* = 0.0001 */)
{
    if ( !m_aacFile->m_decoder )
        return;

    silenceThreshold *= static_cast<double>( std::numeric_limits<short>::max() );

    for (;;)
    {
        if ( m_aacFile->m_header == AAC_File::AAC_MP4 )
        {
            if ( !static_cast<AAC_MP4_File*>(m_aacFile)->readSample() )
                break;
        }
        NeAACDecFrameInfo frameInfo;

        void* sampleBuffer = NeAACDecDecode(m_aacFile->m_decoder, &frameInfo, m_aacFile->m_inBuf, static_cast<uint32_t>(m_aacFile->m_inBufSize) );

        m_aacFile->postDecode( frameInfo.bytesconsumed );

        if ( frameInfo.error > 0 )
        {
            break;
        }
        else if ( frameInfo.samples > 0 )
        {
            double sum = 0;
            int16_t *buf = static_cast<int16_t*>(sampleBuffer);
            switch ( frameInfo.channels )
            {
                case 1:
                    for (size_t j = 0; j < frameInfo.samples; ++j)
                        sum += abs( buf[j] );
                    break;
                case 2:
                    for (size_t j = 0; j < frameInfo.samples; j+=2)
                        sum += abs( (buf[j] >> 1) + (buf[j+1] >> 1) );
                    break;
            }
            if ( (sum >= silenceThreshold * static_cast<short>(frameInfo.samples/frameInfo.channels) ) )
                break;
        }
    }
}


int AacSource::updateBuffer( signed short *pBuffer, size_t bufferSize )
{
    size_t nwrit = 0; //number of samples written to the output buffer

    if ( m_aacFile->m_overflowSize > 0 )
    {
        size_t samples_to_use = bufferSize < m_aacFile->m_overflowSize ? bufferSize : m_aacFile->m_overflowSize;
        memcpy( pBuffer, m_aacFile->m_overflow, samples_to_use * sizeof(signed short) );
        nwrit += samples_to_use;
        m_aacFile->m_overflowSize -= samples_to_use;
        memmove( (void*)(m_aacFile->m_overflow), (void*)(m_aacFile->m_overflow + samples_to_use*sizeof(signed short)), samples_to_use*sizeof(signed short) );
    }

    if ( !m_aacFile->m_decoder )
        return 0;

    for (;;)
    {
        signed short* pBufferIt = pBuffer + nwrit;
        void* sampleBuffer;

        assert( nwrit <= bufferSize );

        if ( m_aacFile->m_header == AAC_File::AAC_MP4 )
        {
            if ( !static_cast<AAC_MP4_File*>(m_aacFile)->readSample() )
            {
                m_eof = true;
                return static_cast<int>(nwrit);
            }
        }
        NeAACDecFrameInfo frameInfo;

        sampleBuffer = NeAACDecDecode(m_aacFile->m_decoder, &frameInfo, m_aacFile->m_inBuf, static_cast<uint32_t>(m_aacFile->m_inBufSize) );
        size_t samples_to_use = (bufferSize - nwrit) < frameInfo.samples ? bufferSize-nwrit : frameInfo.samples;

        if ( samples_to_use > 0 && sampleBuffer != NULL )
        {
            memcpy( pBufferIt, sampleBuffer, samples_to_use * sizeof(signed short) );
            nwrit += samples_to_use;
        }

        if ( samples_to_use < frameInfo.samples )
        {
            m_aacFile->m_overflow = static_cast<unsigned char*>(realloc( m_aacFile->m_overflow, (frameInfo.samples - samples_to_use) * sizeof(signed short) ) );
            memcpy( m_aacFile->m_overflow, static_cast<signed short*>(sampleBuffer) + samples_to_use, (frameInfo.samples - samples_to_use) * sizeof(signed short) );
            m_aacFile->m_overflowSize = frameInfo.samples - samples_to_use;
        }

        m_aacFile->postDecode( frameInfo.bytesconsumed );

        if ( sampleBuffer == NULL )
        {
            m_eof = true;
            break;
        }

        if ( frameInfo.error > 0 )
        {
            std::cerr << "Error: " << NeAACDecGetErrorMessage(frameInfo.error) << std::endl;
            break;
        }

        if ( nwrit == bufferSize )
            break;
   }

   return static_cast<int>(nwrit);
}
