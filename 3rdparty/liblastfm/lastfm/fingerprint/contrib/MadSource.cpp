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

#include <iostream>
#include <fstream>
#include <limits>
#include <climits>
#include <cstdlib>
#include <sstream>
#include <cassert>
#include <stdexcept>
#include "MadSource.h"

#undef max // was definded in mad

using namespace std;


// -----------------------------------------------------------

MadSource::MadSource()
          : m_pMP3_Buffer ( new unsigned char[m_MP3_BufferSize+MAD_BUFFER_GUARD] )
{}

// -----------------------------------------------------------

MadSource::~MadSource()
{
   if ( m_inputFile.isOpen() )
   {
      m_inputFile.close();
      mad_synth_finish(&m_mad_synth);
      mad_frame_finish(&m_mad_frame);
      mad_stream_finish(&m_mad_stream);
   }
   if (m_pMP3_Buffer) delete[] m_pMP3_Buffer;
}

// ---------------------------------------------------------------------

inline short f2s(mad_fixed_t f)
{
   /* A fixed point number is formed of the following bit pattern:
   *
   * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
   * MSB                          LSB
   * S ==> Sign (0 is positive, 1 is negative)
   * W ==> Whole part bits
   * F ==> Fractional part bits
   *
   * This pattern contains MAD_F_FRACBITS fractional bits, one
   * should alway use this macro when working on the bits of a fixed
   * point number. It is not guaranteed to be constant over the
   * different platforms supported by libmad.
   *
   * The signed short value is formed, after clipping, by the least
   * significant whole part bit, followed by the 15 most significant
   * fractional part bits. Warning: this is a quick and dirty way to
   * compute the 16-bit number, madplay includes much better
   * algorithms.
   */

   /* Clipping */
   if(f >= MAD_F_ONE)
      return(SHRT_MAX);
   if(f <= -MAD_F_ONE)
      return(-SHRT_MAX);

   /* Conversion. */
   f = f >> (MAD_F_FRACBITS-15);
   return (signed short)f;
}

// ---------------------------------------------------------------------

string MadSource::MadErrorString(const mad_error& error)
{
   switch(error)
   {
      /* Generic unrecoverable errors. */
   case MAD_ERROR_BUFLEN:
      return("input buffer too small (or EOF)");
   case MAD_ERROR_BUFPTR:
      return("invalid (null) buffer pointer");
   case MAD_ERROR_NOMEM:
      return("not enough memory");

      /* Frame header related unrecoverable errors. */
   case MAD_ERROR_LOSTSYNC:
      return("lost synchronization");
   case MAD_ERROR_BADLAYER:
      return("reserved header layer value");
   case MAD_ERROR_BADBITRATE:
      return("forbidden bitrate value");
   case MAD_ERROR_BADSAMPLERATE:
      return("reserved sample frequency value");
   case MAD_ERROR_BADEMPHASIS:
      return("reserved emphasis value");

      /* Recoverable errors */
   case MAD_ERROR_BADCRC:
      return("CRC check failed");
   case MAD_ERROR_BADBITALLOC:
      return("forbidden bit allocation value");
   case MAD_ERROR_BADSCALEFACTOR:
      return("bad scalefactor index");
   case MAD_ERROR_BADFRAMELEN:
      return("bad frame length");
   case MAD_ERROR_BADBIGVALUES:
      return("bad big_values count");
   case MAD_ERROR_BADBLOCKTYPE:
      return("reserved block_type");
   case MAD_ERROR_BADSCFSI:
      return("bad scalefactor selection info");
   case MAD_ERROR_BADDATAPTR:
      return("bad main_data_begin pointer");
   case MAD_ERROR_BADPART3LEN:
      return("bad audio data length");
   case MAD_ERROR_BADHUFFTABLE:
      return("bad Huffman table select");
   case MAD_ERROR_BADHUFFDATA:
      return("Huffman data overrun");
   case MAD_ERROR_BADSTEREO:
      return("incompatible block_type for JS");

      /* Unknown error. This switch may be out of sync with libmad's
      * defined error codes.
      */
   default:
      return("Unknown error code");
   }
}


// -----------------------------------------------------------------------------

bool MadSource::isRecoverable(const mad_error& error, bool log)
{
   if (MAD_RECOVERABLE (error))
   {
      /* Do not print a message if the error is a loss of
      * synchronization and this loss is due to the end of
      * stream guard bytes. (See the comments marked {3}
      * supra for more informations about guard bytes.)
      */
      if (error != MAD_ERROR_LOSTSYNC   /*|| mad_stream.this_frame != pGuard */ && log)
      {
         cerr << "Recoverable frame level error: " 
              << MadErrorString(error) << endl;
      }

      return true;
   }
   else
   {
      if (error == MAD_ERROR_BUFLEN)
         return true;
      else
      {
         stringstream ss;

         ss << "Unrecoverable frame level error: " 
            << MadErrorString (error) << endl;
         throw ss.str();
      }
   }

   return false;
}

// -----------------------------------------------------------

void MadSource::init(const QString& fileName)
{
   m_inputFile.setFileName( m_fileName = fileName );
   bool fine = m_inputFile.open( QIODevice::ReadOnly );

   if ( !fine )
   {
      throw std::runtime_error ("Cannot load mp3 file!");
   }

   mad_stream_init(&m_mad_stream);
   mad_frame_init (&m_mad_frame);
   mad_synth_init (&m_mad_synth);
   mad_timer_reset(&m_mad_timer);

   m_pcmpos = m_mad_synth.pcm.length;
}

// -----------------------------------------------------------------------------

/*QString MadSource::getMbid()
{
    char out[MBID_BUFFER_SIZE];
    int const r = getMP3_MBID( QFile::encodeName( m_fileName ), out );
    if (r == 0)
        return QString::fromLatin1( out );
    return QString();
}*/

void MadSource::getInfo(int& lengthSecs, int& samplerate, int& bitrate, int& nchannels )
{
   // get the header plus some other stuff..
   QFile inputFile(m_fileName);
   bool fine = inputFile.open( QIODevice::ReadOnly );

   if ( !fine )
   {
      throw std::runtime_error ("ERROR: Cannot load file for getInfo!");
      return;
   }

   unsigned char* pMP3_Buffer  = new unsigned char[m_MP3_BufferSize+MAD_BUFFER_GUARD];

   mad_stream   madStream;
   mad_header  madHeader;
   mad_timer_t  madTimer;

   mad_stream_init(&madStream);
   mad_timer_reset(&madTimer);

   double avgSamplerate = 0;
   double avgBitrate = 0;
   double avgNChannels = 0;
   int nFrames = 0;

   while ( fetchData( inputFile, pMP3_Buffer, m_MP3_BufferSize, madStream) )
   {
      if ( mad_header_decode(&madHeader, &madStream) != 0 )
      {
         if ( isRecoverable(madStream.error) )
            continue;
         else
            break;
      }

      mad_timer_add(&madTimer, madHeader.duration);

      avgSamplerate += madHeader.samplerate;
      avgBitrate += madHeader.bitrate;

      if ( madHeader.mode == MAD_MODE_SINGLE_CHANNEL )
         ++avgNChannels;
      else
         avgNChannels += 2;

      ++nFrames;
   }

   inputFile.close();
   mad_stream_finish(&madStream);
   mad_header_finish(&madHeader);
   delete[] pMP3_Buffer;


   lengthSecs = static_cast<int>(madTimer.seconds);
   samplerate = static_cast<int>( (avgSamplerate/nFrames) + 0.5 );
   bitrate = static_cast<int>( (avgBitrate/nFrames) + 0.5 );
   nchannels = static_cast<int>( (avgNChannels/nFrames) + 0.5 );
}

// -----------------------------------------------------------


bool MadSource::fetchData( QFile& mp3File,
                            unsigned char* pMP3_Buffer,
                            const int MP3_BufferSize,
                            mad_stream& madStream )
{
   unsigned char *pReadStart = NULL;
   unsigned char *pGuard = NULL;

   if ( madStream.buffer == NULL || 
        madStream.error == MAD_ERROR_BUFLEN )
   {

      size_t readSize;
      size_t remaining;

      /* {2} libmad may not consume all bytes of the input
      * buffer. If the last frame in the buffer is not wholly
      * contained by it, then that frame's start is pointed by
      * the next_frame member of the Stream structure. This
      * common situation occurs when mad_frame_decode() fails,
      * sets the stream error code to MAD_ERROR_BUFLEN, and
      * sets the next_frame pointer to a non NULL value. (See
      * also the comment marked {4} bellow.)
      *
      * When this occurs, the remaining unused bytes must be
      * put back at the beginning of the buffer and taken in
      * account before refilling the buffer. This means that
      * the input buffer must be large enough to hold a whole
      * frame at the highest observable bit-rate (currently 448
      * kb/s). XXX=XXX Is 2016 bytes the size of the largest
      * frame? (448000*(1152/32000))/8
      */
      if (madStream.next_frame != NULL)
      {
         remaining = madStream.bufend - madStream.next_frame;
         memmove (pMP3_Buffer, madStream.next_frame, remaining);

         pReadStart = pMP3_Buffer + remaining;
         readSize = MP3_BufferSize - remaining;
      }
      else
      {
         readSize = MP3_BufferSize;
         pReadStart = pMP3_Buffer;
         remaining = 0;
      }

      readSize = mp3File.read( reinterpret_cast<char*>(pReadStart), readSize );

      // nothing else to read!
      if (readSize <= 0) 
         return false;

      if ( mp3File.atEnd() )
      {
         pGuard = pReadStart + readSize;

         memset (pGuard, 0, MAD_BUFFER_GUARD);
         readSize += MAD_BUFFER_GUARD;
      }

      // Pipe the new buffer content to libmad's stream decoder facility.
      mad_stream_buffer( &madStream, pMP3_Buffer,
                         static_cast<unsigned int>(readSize + remaining));

      madStream.error = MAD_ERROR_NONE;
   }

   return true;
}

// -----------------------------------------------------------------------------

void MadSource::skipSilence(double silenceThreshold /* = 0.0001 */)
{
   mad_frame  madFrame;
   mad_synth    madSynth;

   mad_frame_init(&madFrame);
   mad_synth_init (&madSynth);

   silenceThreshold *= static_cast<double>( numeric_limits<short>::max() );

   for (;;)
   {
      if ( !fetchData( m_inputFile, m_pMP3_Buffer, m_MP3_BufferSize, m_mad_stream) )
         break;

      if ( mad_frame_decode(&madFrame, &m_mad_stream) != 0 )
      {
         if ( isRecoverable(m_mad_stream.error) )
            continue;
         else
            break;
      }

      mad_synth_frame (&madSynth, &madFrame);

      double sum = 0;

      switch (madSynth.pcm.channels)
      {
      case 1:
         for (size_t j = 0; j < madSynth.pcm.length; ++j)
            sum += abs(f2s(madSynth.pcm.samples[0][j]));
         break;
      case 2:
         for (size_t j = 0; j < madSynth.pcm.length; ++j)
            sum += abs(f2s(
                     (madSynth.pcm.samples[0][j] >> 1)
                   + (madSynth.pcm.samples[1][j] >> 1)));
         break;
      }

      if ( (sum >= silenceThreshold * madSynth.pcm.length) )
         break;
   }

   mad_frame_finish(&madFrame);
}

// -----------------------------------------------------------------------------

void MadSource::skip(const int mSecs)
{
   if ( mSecs <= 0 )
      return;

   mad_header  madHeader;
   mad_header_init(&madHeader);

   for (;;)
   {
      if (!fetchData( m_inputFile, m_pMP3_Buffer, m_MP3_BufferSize, m_mad_stream))
         break;

      if ( mad_header_decode(&madHeader, &m_mad_stream) != 0 )
      {
         if ( isRecoverable(m_mad_stream.error) )
            continue;
         else
            break;
      }
 
      mad_timer_add(&m_mad_timer, madHeader.duration);

      if ( mad_timer_count(m_mad_timer, MAD_UNITS_MILLISECONDS) >= mSecs )
         break;
   }

   mad_header_finish(&madHeader);
}

// -----------------------------------------------------------

int MadSource::updateBuffer(signed short* pBuffer, size_t bufferSize)
{
   size_t nwrit = 0; //number of samples written to the output buffer

   for (;;)
   {
      // get a (valid) frame
      // m_pcmpos == 0 could mean two things
      // - we have completely decoded a frame, but the output buffer is still
      //   not full (it would make more sense for pcmpos == pcm.length(), but
      //   the loop assigns pcmpos = 0 at the end and does it this way!
      // - we are starting a stream
      if ( m_pcmpos == m_mad_synth.pcm.length )
      {
         if ( !fetchData( m_inputFile, m_pMP3_Buffer, m_MP3_BufferSize, m_mad_stream) )
         {
            break; // nothing else to read
         }

         // decode the frame
         if (mad_frame_decode (&m_mad_frame, &m_mad_stream))
         {
            if ( isRecoverable(m_mad_stream.error) )
               continue;
            else
               break;
         } // if (mad_frame_decode (&madFrame, &madStream))

         mad_timer_add (&m_mad_timer, m_mad_frame.header.duration);
         mad_synth_frame (&m_mad_synth, &m_mad_frame);

         m_pcmpos = 0;
      }

      size_t samples_for_mp3 = m_mad_synth.pcm.length - m_pcmpos;
      size_t samples_for_buf = bufferSize - nwrit;
      signed short* pBufferIt = pBuffer + nwrit;
      size_t i = 0, j = 0;

      switch( m_mad_synth.pcm.channels )
      {
      case 1:
         {
            size_t samples_to_use = min (samples_for_mp3, samples_for_buf);
            for (i = 0; i < samples_to_use; ++i )
               pBufferIt[i] = f2s( m_mad_synth.pcm.samples[0][i+m_pcmpos] );
         }
         j = i;
         break;

      case 2:
         for (; i < samples_for_mp3 && j < samples_for_buf ; ++i, j+=2 )
         {
            pBufferIt[j]   = f2s( m_mad_synth.pcm.samples[0][i+m_pcmpos] );
            pBufferIt[j+1] = f2s( m_mad_synth.pcm.samples[1][i+m_pcmpos] );
         }
         break;

      default:
         cerr << "wtf kind of mp3 has " << m_mad_synth.pcm.channels << " channels??\n";
         break;
      }

      m_pcmpos += i;
      nwrit += j;

      assert( nwrit <= bufferSize );

      if (nwrit == bufferSize) 
         return static_cast<int>(nwrit);
   }

   return static_cast<int>(nwrit);
}

// -----------------------------------------------------------------------------

