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

#ifndef __MP3_SOURCE_H__
#define __MP3_SOURCE_H__

#include <lastfm/FingerprintableSource>
#include <QFile>
#include <string>
#include <vector>
#include <fstream>
#include <mad.h>


class MadSource : public lastfm::FingerprintableSource
{
public:
    MadSource();
    ~MadSource();

    virtual void getInfo(int& lengthSecs, int& samplerate, int& bitrate, int& nchannels);
    virtual void init(const QString& fileName);
    virtual int updateBuffer(signed short* pBuffer, size_t bufferSize);
    virtual void skip(const int mSecs);
    virtual void skipSilence(double silenceThreshold = 0.0001);
    virtual bool eof() const { return m_inputFile.atEnd(); }

private:
    static bool fetchData( QFile& mp3File,
                           unsigned char* pMP3_Buffer,
                           const int MP3_BufferSize,
                           mad_stream& madStream );

    static bool isRecoverable(const mad_error& error, bool log = false);

    static std::string MadErrorString(const mad_error& error);

    struct mad_stream    m_mad_stream;
    struct mad_frame     m_mad_frame;
    mad_timer_t          m_mad_timer;
    struct mad_synth     m_mad_synth;

    QFile                m_inputFile;

    unsigned char*       m_pMP3_Buffer;
    static const int     m_MP3_BufferSize = (5*8192);
    QString              m_fileName;

    size_t               m_pcmpos;
};

#endif
