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
#include <faad.h>
#include <mp4ff.h>

class AAC_File
{
public:
    AAC_File(const QString&, int headerType);
    virtual ~AAC_File();
    virtual void getInfo( int& lengthSecs, int& samplerate, int& bitrate, int& nchannels ) = 0;
    virtual bool init() = 0;
    //virtual QString getMbid() = 0;
    virtual void skip( const int mSecs ) = 0;
    virtual void postDecode(unsigned long) = 0;

    enum HeaderType
    {
        AAC_UNKNOWN = 0,
        AAC_ADIF,
        AAC_ADTS,
        AAC_MP4
    };

    QString m_fileName;
    unsigned char *m_inBuf;
    size_t m_inBufSize;
    NeAACDecHandle m_decoder;
    unsigned char *m_overflow;
    size_t m_overflowSize;
    int m_header;
};


class AAC_MP4_File : public AAC_File
{
public:
    AAC_MP4_File(const QString&, int headerType = AAC_MP4 );
    ~AAC_MP4_File();
    virtual void getInfo( int& lengthSecs, int& samplerate, int& bitrate, int& nchannels );
    virtual bool init();
    //virtual QString getMbid();
    virtual void skip( const int mSecs );
    virtual void postDecode(unsigned long);
    int32_t readSample();

private:
    bool commonSetup( NeAACDecHandle& handle, mp4ff_callback_t*& cb, FILE*& fp, mp4ff_t*& mp4, int32_t& audioTrack );
    virtual int32_t getTrack( const mp4ff_t* f );
    int m_mp4AudioTrack;
    uint32_t m_mp4SampleId;
    mp4ff_t *m_mp4File;
    mp4ff_callback_t *m_mp4cb;
};


class AAC_ADTS_File : public AAC_File
{
public:
    AAC_ADTS_File( const QString& fileName, int headerType );
    ~AAC_ADTS_File();
    virtual void getInfo( int& lengthSecs, int& samplerate, int& bitrate, int& nchannels );
    virtual bool init();
    //virtual QString getMbid();
    virtual void skip( const int mSecs );
    virtual void postDecode(unsigned long bytesconsumed );

private:
    int32_t commonSetup( FILE*& fp, NeAACDecHandle& decoder, unsigned char*& buf, size_t& bufSize, uint32_t& samplerate, uint8_t& channels );
    void parse( FILE*& fp, unsigned char*& buf, size_t& bufSize, int &bitrate, double &length );
    void fillBuffer( FILE*& fp, unsigned char*& buf, size_t& bufSize, const size_t m_bytesConsumed );

    FILE* m_file;
    // These two only needed for skipping AAC ADIF files
    uint32_t m_adifSamplerate;
    int m_adifChannels;
};
