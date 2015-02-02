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
#ifndef __AAC_SOURCE_H__
#define __AAC_SOURCE_H__

#include <lastfm/FingerprintableSource>

class AacSource : public lastfm::FingerprintableSource
{
public:
    AacSource();
    ~AacSource();

    virtual void getInfo(int& lengthSecs, int& samplerate, int& bitrate, int& nchannels);
    virtual void init(const QString& fileName);
    virtual int updateBuffer(signed short* pBuffer, size_t bufferSize);
    virtual void skip(const int mSecs);
    virtual void skipSilence(double silenceThreshold = 0.0001);
    virtual bool eof() const { return m_eof; }

private:
    int checkHeader();
    QString m_fileName;
    bool m_eof;
    class AAC_File *m_aacFile;
};

#endif

