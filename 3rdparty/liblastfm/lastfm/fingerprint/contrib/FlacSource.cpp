/*
   Copyright 2009 Last.fm Ltd. 
   Copyright 2009 John Stamp <jstamp@users.sourceforge.net>

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
#include "FlacSource.h"
#include <algorithm>
#include <cassert>
#include <errno.h>
#include <iostream>
#include <limits>
#include <algorithm>
#include <stdexcept>

#include <QFile>


FLAC__StreamDecoderWriteStatus FlacSource::_write_callback(const FLAC__StreamDecoder *, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
    assert(client_data != NULL);
    FlacSource *instance = reinterpret_cast<FlacSource *>(client_data);
    assert(instance != NULL);
    return instance->write_callback(frame, buffer);
}

FLAC__StreamDecoderWriteStatus FlacSource::write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    m_outBufLen = 0;

    if ( m_outBuf )
    {
        size_t i;
        for(i = 0; i < frame->header.blocksize; i++)
        {
            switch ( m_channels )
            {
                case 1:
                    m_outBuf[m_outBufLen] = (FLAC__int16)buffer[0][i]; // mono
                    m_outBufLen++;
                    break;
                case 2:
                    m_outBuf[m_outBufLen] = (FLAC__int16)buffer[0][i]; // left channel
                    m_outBuf[m_outBufLen+1] = (FLAC__int16)buffer[1][i]; // right channel
                    m_outBufLen += 2;
                    break;
            }
        }
        m_samplePos += frame->header.blocksize;
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

// ---------------------------------------------------------------------

void FlacSource::_metadata_callback(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *client_data)
{
    assert(client_data != NULL);
    FlacSource *instance = reinterpret_cast<FlacSource *>(client_data);
    assert(instance != NULL);
    instance->metadata_callback(metadata);
}

void FlacSource::metadata_callback( const FLAC__StreamMetadata *metadata )
{
    switch ( metadata->type )
    {
        case FLAC__METADATA_TYPE_STREAMINFO:
            m_channels = metadata->data.stream_info.channels;
            m_totalSamples = metadata->data.stream_info.total_samples;
            m_samplerate = metadata->data.stream_info.sample_rate;
            m_bps = metadata->data.stream_info.bits_per_sample;
            m_maxFrameSize = metadata->data.stream_info.max_framesize;
            break;
        case FLAC__METADATA_TYPE_VORBIS_COMMENT:
            m_commentData = FLAC__metadata_object_clone(metadata);
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------

void FlacSource::_error_callback(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    assert(client_data != NULL);
    FlacSource *instance = reinterpret_cast<FlacSource *>(client_data);
    assert(instance != NULL);
    instance->error_callback(status);
}

void FlacSource::error_callback(FLAC__StreamDecoderErrorStatus status)
{
    std::cerr << "Got FLAC error: " << FLAC__StreamDecoderErrorStatusString[status] << std::endl;
}

// ---------------------------------------------------------------------

FlacSource::FlacSource()
    : m_decoder( 0 )
    , m_fileSize( 0 )
    , m_outBuf( 0 )
    , m_outBufLen( 0 )
    , m_outBufPos( 0 )
    , m_samplePos( 0 )
    , m_maxFrameSize( 0 )
    , m_commentData( 0 )
    , m_bps( 0 )
    , m_channels( 0 )
    , m_samplerate( 0 )
    , m_totalSamples( 0 )
    , m_eof( false )
{
}

// ---------------------------------------------------------------------

FlacSource::~FlacSource()
{
    if ( m_decoder )
    {
        FLAC__stream_decoder_finish( m_decoder );
        FLAC__stream_decoder_delete( m_decoder );
    }
    if ( m_commentData )
        FLAC__metadata_object_delete( m_commentData );
    if ( m_outBuf )
        free( m_outBuf );
}

// ---------------------------------------------------------------------

void FlacSource::init(const QString& fileName)
{
    m_fileName = fileName;
    
    if ( !m_decoder )
    {
        FILE *f = fopen(QFile::encodeName(m_fileName), "rb" );
        if ( f )
        {
            // Need to check which init call to use; flac doesn't do that for us
            unsigned char header[35];
            bool isOgg = false;
            fread( header, 1, 35, f );
            if ( memcmp(header, "OggS", 4) == 0 &&
                 memcmp(&header[29], "FLAC", 4) == 0 )
                isOgg = true;

            // getInfo() will need this to calculate bitrate
            fseek( f, 0, SEEK_END );
            m_fileSize = ftell(f);

            rewind( f );

            m_decoder = FLAC__stream_decoder_new();
            FLAC__stream_decoder_set_metadata_respond(m_decoder, FLAC__METADATA_TYPE_STREAMINFO);
            FLAC__stream_decoder_set_metadata_respond(m_decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

            int init_status;
            if ( FLAC_API_SUPPORTS_OGG_FLAC && isOgg )
                init_status = FLAC__stream_decoder_init_ogg_FILE( m_decoder, f, _write_callback, _metadata_callback, _error_callback, this );
            else
                init_status = FLAC__stream_decoder_init_FILE( m_decoder, f, _write_callback, _metadata_callback, _error_callback, this );

            if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
                return;

            FLAC__stream_decoder_process_until_end_of_metadata( m_decoder );
            m_outBuf = static_cast<signed short*>(malloc( sizeof(signed short)*m_maxFrameSize));

            if ( m_bps != 16 )
            {
                FLAC__stream_decoder_finish( m_decoder );
                FLAC__stream_decoder_delete( m_decoder );
                FLAC__metadata_object_delete( m_commentData );
                m_decoder = 0;
                m_commentData = 0;
                throw std::runtime_error( "ERROR: only 16 bit FLAC files are currently supported!" );
            }
        }
        else
            throw std::runtime_error( "ERROR: cannot load FLAC file!" );
    }
}

// ---------------------------------------------------------------------

/*QString FlacSource::getMbid()
{
    if ( m_commentData )
    {
        FLAC__StreamMetadata_VorbisComment *vc;
        vc = &m_commentData->data.vorbis_comment;
        for ( unsigned int i = 0; i < vc->num_comments; ++i )
        {
            QByteArray key( (char*)(vc->comments[i].entry), vc->comments[i].length );
            if ( key.left(20).toLower() == "musicbrainz_trackid=" )
            {
                QString val = key.mid(20);
                return val;
            }
        }
    }

    return QString();
}*/

// ---------------------------------------------------------------------

void FlacSource::getInfo(int& lengthSecs, int& samplerate, int& bitrate, int& nchannels)
{
    lengthSecs = 0;
    samplerate = 0;
    bitrate = 0;
    nchannels = 0;

    if ( m_decoder )
    {
        samplerate = m_samplerate;
        nchannels = m_channels;
        if ( samplerate > 0 )
            lengthSecs = static_cast<int>( static_cast<double>(m_totalSamples)/m_samplerate + 0.5);

        // Calcuate bitrate
        if ( lengthSecs > 0 )
        {
            FLAC__Metadata_SimpleIterator *it = FLAC__metadata_simple_iterator_new();
            FLAC__metadata_simple_iterator_init( it, QFile::encodeName(m_fileName), true, true );
            while( !FLAC__metadata_simple_iterator_is_last( it ) )
            {
                FLAC__metadata_simple_iterator_next( it );
            }
            off_t audioOffset = FLAC__metadata_simple_iterator_get_block_offset( it ) +
                FLAC__metadata_simple_iterator_get_block_length( it );
            FLAC__metadata_simple_iterator_delete( it );
            bitrate = static_cast<int>( static_cast<double>(m_fileSize - audioOffset) * 8 / lengthSecs + 0.5 );
        }
    }
}

// ---------------------------------------------------------------------

void FlacSource::skip( const int mSecs )
{
    FLAC__uint64 absSample = mSecs * m_samplerate / 1000 + m_samplePos;
    if ( !FLAC__stream_decoder_seek_absolute(m_decoder, absSample) )
        FLAC__stream_decoder_reset( m_decoder );
    m_outBufLen = 0;
}

// ---------------------------------------------------------------------

void FlacSource::skipSilence(double silenceThreshold /* = 0.0001 */)
{
    silenceThreshold *= static_cast<double>( std::numeric_limits<short>::max() );
    for ( ;; )
    {
        double sum = 0;
        bool result = FLAC__stream_decoder_process_single( m_decoder );
        // there was a fatal read
        if ( !result )
            break;

        switch ( m_channels )
        {
            case 1:
                for (size_t j = 0; j < m_outBufLen; ++j)
                    sum += abs( m_outBuf[j] );
                break;
            case 2:
                for ( size_t j = 0; j < m_outBufLen; j+=2 )
                    sum += abs( (m_outBuf[j] >> 1)
                                + (m_outBuf[j+1] >> 1) );
                break;
        }
        if ( (sum >= silenceThreshold * static_cast<double>(m_outBufLen) ) )
            break;
    }
    m_outBufLen = 0;
}

// ---------------------------------------------------------------------

int FlacSource::updateBuffer( signed short *pBuffer, size_t bufferSize )
{
    size_t nwrit = 0;

    for ( ;; )
    {
        size_t samples_to_use = std::min (bufferSize - nwrit, m_outBufLen - m_outBufPos);
        signed short* pBufferIt = pBuffer + nwrit;

        nwrit += samples_to_use;
        assert( nwrit <= bufferSize );
        memcpy( pBufferIt, m_outBuf + m_outBufPos, sizeof(signed short)*samples_to_use );

        if ( samples_to_use < m_outBufLen - m_outBufPos )
            m_outBufPos = samples_to_use;
        else
        {
            m_outBufPos = 0;
            bool result = FLAC__stream_decoder_process_single( m_decoder );
            // there was a fatal read
            if ( !result )
            {
                std::cerr << "Fatal error decoding FLAC" << std::endl;
                return 0;
            }
            else if ( FLAC__stream_decoder_get_state( m_decoder ) == FLAC__STREAM_DECODER_END_OF_STREAM )
            {
                m_eof = true;
                break;
            }
        }

        if ( nwrit == bufferSize )
            return static_cast<int>(nwrit);
    }
    return static_cast<int>(nwrit);
}

// -----------------------------------------------------------------------------

