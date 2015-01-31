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
#ifndef __FLAC_SOURCE_H__
#define __FLAC_SOURCE_H__

#include <lastfm/FingerprintableSource>
#include <FLAC/stream_decoder.h>
#include <FLAC/metadata.h>


class FlacSource : public lastfm::FingerprintableSource
{
public:
    FlacSource();
    virtual ~FlacSource();

    virtual void getInfo(int& lengthSecs, int& samplerate, int& bitrate, int& nchannels);
    virtual void init(const QString& fileName);

    // return a chunk of PCM data from the FLAC file
    virtual int updateBuffer(signed short* pBuffer, size_t bufferSize);

    virtual void skip(const int mSecs);
    virtual void skipSilence(double silenceThreshold = 0.0001);

    //QString getMbid();

    bool eof() const { return m_eof; }

private:
    static FLAC__StreamDecoderWriteStatus _write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
    static void _metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
    static void _error_callback(const ::FLAC__StreamDecoder *decoder, ::FLAC__StreamDecoderErrorStatus status, void *client_data);

    FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
    void metadata_callback( const FLAC__StreamMetadata *metadata );
    void error_callback(FLAC__StreamDecoderErrorStatus status);

    FLAC__StreamDecoder *m_decoder;
    QString m_fileName;
    size_t m_fileSize;
    short *m_outBuf;
    size_t m_outBufLen;
    size_t m_outBufPos;
    FLAC__uint64 m_samplePos;
    unsigned m_maxFrameSize;
    FLAC__StreamMetadata* m_commentData;
    unsigned m_bps;
    unsigned m_channels;
    unsigned m_samplerate;
    FLAC__uint64 m_totalSamples;

    bool m_eof;
};

#endif

