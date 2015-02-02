/*
   Copyright 2005-2009 Last.fm Ltd. <mir@last.fm>

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
#include <limits>
#include <bitset>
#include <deque>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <cstring>

#include <samplerate.h> // libsamplerate

#include "FingerprintExtractor.h"
#include "fp_helper_fun.h" // for GroupData
#include "Filter.h"
#include "FloatingAverage.h"
#include "OptFFT.h"

//////////////////////////////////////////////////////////////////////////

namespace fingerprint
{

using namespace std;
static const int NUM_FRAMES_CLIENT = 32; // ~= 10 secs.

enum eProcessType
{
   PT_UNKNOWN,
   PT_FOR_QUERY,
   PT_FOR_FULLSUBMIT
};

//////////////////////////////////////////////////////////////////////////

class PimplData
{

public:

   PimplData()
   : m_pDownsampledPCM(NULL), m_pDownsampledCurrIt(NULL),
     m_normalizedWindowMs(static_cast<unsigned int>(NORMALIZATION_SKIP_SECS * 1000 * 2)),
     m_compensateBufferSize(FRAMESIZE-OVERLAPSAMPLES + Filter::KEYWIDTH * OVERLAPSAMPLES),
     m_downsampledProcessSize(NUM_FRAMES_CLIENT*FRAMESIZE),
     // notice that the buffer has extra space on either side for the normalization window  
     m_fullDownsampledBufferSize( m_downsampledProcessSize + // the actual processed part
                                  m_compensateBufferSize +  // a compensation buffer for the fft
                                ((m_normalizedWindowMs * DFREQ / 1000) / 2) ), // a compensation buffer for the normalization
     m_normWindow(m_normalizedWindowMs * DFREQ / 1000),
     m_pFFT(NULL), m_pDownsampleState(NULL), m_processType(PT_UNKNOWN)
   {
      m_pFFT            = new OptFFT(m_downsampledProcessSize + m_compensateBufferSize);
      m_pDownsampledPCM = new float[m_fullDownsampledBufferSize];

      // the end of ||-------m_bufferSize-------|-cb-|---norm/2---|| 
      //                                                           ^-- pEndDownsampledBuf
      m_pEndDownsampledBuf = m_pDownsampledPCM + m_fullDownsampledBufferSize;

      // loading filters
      size_t numFilters = sizeof(rFilters) / sizeof(RawFilter) ;
      for (size_t i = 0; i < numFilters; ++i)
         m_filters.push_back( Filter( rFilters[i].ftid, rFilters[i].thresh, rFilters[i].weight ) );

   }

   ~PimplData()
   {
      if ( m_pFFT )
         delete m_pFFT;
      m_pFFT = NULL;
      if ( m_pDownsampledPCM )
         delete [] m_pDownsampledPCM;
      m_pDownsampledPCM = NULL;

      if ( m_pDownsampleState )
         src_delete(m_pDownsampleState) ;

   }

   float*                 m_pDownsampledPCM;
   float*                 m_pDownsampledCurrIt;

   const unsigned int     m_normalizedWindowMs;
   const size_t           m_compensateBufferSize;
   const size_t           m_downsampledProcessSize;
   const size_t           m_fullDownsampledBufferSize;

   FloatingAverage<double> m_normWindow;
   OptFFT*                 m_pFFT;

   //////////////////////////////////////////////////////////////////////////
   
   // libsamplerate
   SRC_STATE*              m_pDownsampleState;
   SRC_DATA                m_downsampleData;

   vector<float>           m_floatInData;

   //////////////////////////////////////////////////////////////////////////


   bool                   m_groupsReady;
   bool                   m_preBufferPassed;

   eProcessType           m_processType;

   size_t                 m_toSkipSize;
   size_t                 m_toSkipMs;

   size_t                 m_skippedSoFar;
   bool                   m_skipPassed;

   float*                 m_pEndDownsampledBuf;

   int m_freq;
   int m_nchannels;

   unsigned int m_lengthMs;
   int          m_minUniqueKeys;
   unsigned int m_uniqueKeyWindowMs;

   unsigned int m_toProcessKeys;
   unsigned int m_totalWindowKeys;
   
   vector<Filter>     m_filters;

   deque<GroupData>   m_groupWindow;
   vector<GroupData>  m_groups;
   unsigned int       m_processedKeys;

   vector<unsigned int>   m_partialBits; // here just to avoid reallocation

#if __BIG_ENDIAN__

#define reorderbits(X)  ((((unsigned int)(X) & 0xff000000) >> 24) | \
                        (((unsigned int)(X) & 0x00ff0000) >> 8)  | \
                        (((unsigned int)(X) & 0x0000ff00) << 8)  | \
                        (((unsigned int)(X) & 0x000000ff) << 24))

   vector<GroupData>  m_bigEndianGroups;
#endif
};

//////////////////////////////////////////////////////////////////////////

void initCustom( PimplData& pd,
                 int freq, int nchannels,
                 unsigned int lengthMs, unsigned int skipMs,
                 int minUniqueKeys, unsigned int uniqueKeyWindowMs, int duration );

inline float getRMS( const FloatingAverage<double>& signal );
unsigned int processKeys( deque<GroupData>& groups, size_t size, PimplData& pd );
void         integralImage( float** ppFrames, unsigned int nFrames );
void         computeBits( vector<unsigned int>& bits,
                          const vector<Filter>& f, 
                          float ** frames, unsigned int nframes );


void src_short_to_float_and_mono_array(const short *in, float *out, int srclen, int nchannels);

//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------

FingerprintExtractor::FingerprintExtractor()
: m_pPimplData(NULL)
{
   m_pPimplData = new PimplData();
}

// -----------------------------------------------------------------------------

FingerprintExtractor::~FingerprintExtractor()
{
   if ( m_pPimplData )
      delete m_pPimplData;
}

// -----------------------------------------------------------------------------

size_t FingerprintExtractor::getToSkipMs() 
{ return m_pPimplData->m_toSkipMs; }

// -----------------------------------------------------------------------------

size_t FingerprintExtractor::getMinimumDurationMs()
{
   return static_cast<size_t>( (QUERY_SIZE_SECS + NORMALIZATION_SKIP_SECS * 2 + GUARD_SIZE_SECS) * 1000 );
}

// -----------------------------------------------------------------------------

size_t FingerprintExtractor::getVersion()
{ return FINGERPRINT_LIB_VERSION; }

// -----------------------------------------------------------------------------

void FingerprintExtractor::initForQuery(int freq, int nchannels, int duration )
{
   m_pPimplData->m_skipPassed = false;
   m_pPimplData->m_processType = PT_FOR_QUERY;

   if ( !m_pPimplData )
      throw std::runtime_error("Not enough RAM to allocate the fingerprinter!");

   initCustom( *m_pPimplData,
               freq, nchannels,
               static_cast<unsigned int>(QUERY_SIZE_SECS * 1000),
               static_cast<unsigned int>(QUERY_START_SECS * 1000), 
               MIN_UNIQUE_KEYS, 
               static_cast<unsigned int>(UPDATE_SIZE_SECS * 1000), duration );
}

// -----------------------------------------------------------------------------

void FingerprintExtractor::initForFullSubmit(int freq, int nchannels )
{
   m_pPimplData->m_skipPassed = true;
   m_pPimplData->m_processType = PT_FOR_FULLSUBMIT;

   if ( !m_pPimplData )
      throw std::runtime_error("Not enough RAM to allocate the fingerprinter!");

   initCustom( *m_pPimplData, 
               freq, nchannels, 
               numeric_limits<unsigned int>::max(), 
               0, MIN_UNIQUE_KEYS, 0, -1 );
}

// -----------------------------------------------------------------------------

void initCustom( PimplData& pd, 
                 int freq, int nchannels,
                 unsigned int lengthMs, 
                 unsigned int skipMs, 
                 int minUniqueKeys, 
                 unsigned int uniqueKeyWindowMs, int duration )
{
   //////////////////////////////////////////////////////////////////////////
   pd.m_freq = freq;
   pd.m_nchannels = nchannels;
   pd.m_lengthMs = lengthMs;
   pd.m_minUniqueKeys = minUniqueKeys;
   pd.m_uniqueKeyWindowMs = uniqueKeyWindowMs;
   //////////////////////////////////////////////////////////////////////////

   // ***********************************************************************
   if ( pd.m_pDownsampleState )
      pd.m_pDownsampleState = src_delete(pd.m_pDownsampleState) ;
   pd.m_pDownsampleState = src_new (SRC_SINC_FASTEST, 1, NULL) ;
   pd.m_downsampleData.src_ratio = FDFREQ / freq;
   // ***********************************************************************

   //////////////////////////////////////////////////////////////////////////
   if ( pd.m_processType == PT_FOR_FULLSUBMIT ) 
      skipMs = 0; // make sure
   else if ( duration > 0 )
   {
      // skip + size + right normalization window + FFT guard
      // 
      int stdDurationMs = static_cast<int>((QUERY_START_SECS + QUERY_SIZE_SECS + NORMALIZATION_SKIP_SECS + GUARD_SIZE_SECS) * 1000);
      int actualDurationMs = duration * 1000;
      // compute the actual skipMs depending on the duration
      if ( actualDurationMs < stdDurationMs )
         skipMs -= max( stdDurationMs - actualDurationMs, 0 );
   }

   pd.m_toSkipMs = max( static_cast<int>(skipMs) - static_cast<int>((pd.m_normalizedWindowMs/2)), 0 );
   pd.m_toSkipSize = static_cast<size_t>( freq * nchannels * 
                                          (pd.m_toSkipMs / 1000.0) ); // half the norm window in secs;

   //if ( pd.m_processType == PT_FOR_QUERY && skipMs > pd.m_normalizedWindowMs/2 )
   //{
   //   pd.m_toSkipMs = skipMs - (pd.m_normalizedWindowMs/2);
   //   pd.m_toSkipSize = static_cast<size_t>( freq * nchannels * 
   //                                         (pd.m_toSkipMs / 1000.0) ); // half the norm window in secs
   //}
   //else
   //{
   //   pd.m_toSkipMs = 0;
   //   pd.m_toSkipSize = 0; // half of the normalization window will be skipped in ANY case
   //}

   pd.m_skippedSoFar = 0;
   pd.m_groupsReady = false;
   pd.m_preBufferPassed = false;

   // prepare the position for pre-buffering
   pd.m_pDownsampledCurrIt = pd.m_pDownsampledPCM + (pd.m_downsampledProcessSize - (pd.m_normWindow.size() / 2) ); 

   pd.m_toProcessKeys = fingerprint::getTotalKeys(pd.m_lengthMs);//  (m_lengthMs * DFREQ) / (1000 * OVERLAPSAMPLES) + 1;
   pd.m_totalWindowKeys = fingerprint::getTotalKeys(pd.m_uniqueKeyWindowMs); //(m_uniqueKeyWindowMs * DFREQ) / (1000 * OVERLAPSAMPLES) + 1;

   if (pd.m_toProcessKeys == 1)
      pd.m_toProcessKeys = 0;
   if (pd.m_totalWindowKeys == 1)
      pd.m_totalWindowKeys = 0;

   pd.m_processedKeys = 0;

   pd.m_groupWindow.clear();
   pd.m_processedKeys = 0;
}

// -----------------------------------------------------------------------------


// * cb = compensate buffer size
// * norm = floating normalization window size
//
// PREBUFFER:
//     (-------m_bufferSize-------)
//    ||    EMPTY    |---norm/2---|-cb-|---norm/2---||
// 1.                {--------read frames-----------}
// 2.                {--read normalize window--}      
// 3.                             {----}   normalize
//
// 1. read [norm + cb] frames to m_bufferSize - norm/2
// 2. read [m_buffersize - norm/2...m_buffersize + norm/2] into normalize window
// 3. normalize [m_bufferSize..m_bufferSize+cb]
//
// PROCESS:
//
//     ||-------m_bufferSize-------|-cb-|---norm/2---||
// 1.   <--------------------------{------copy-------}
// 2.                    {--------read frames-------}
// 3.        {---------normalize--------}
// 4.   {------fft/process/whatevs------}
//
// 1. copy [m_bufferSize..m_bufferSize + cb + norm/2] to beginning
// 2. read m_bufferSize frames to cb + norm/2
// 3. normalize [cb..m_bufferSize+cb]
// 4. fft/process/whatevs [0...m_bufferSize+cb]
//
// repeat until enough blocks processed and enough groups!
//
bool FingerprintExtractor::process( const short* pPCM, size_t num_samples, bool end_of_stream )
{
   if ( num_samples == 0 )
      return false;

   // easier read
   PimplData& pd = *m_pPimplData;

   if ( pd.m_processType == PT_UNKNOWN )
      throw std::runtime_error("Please call initForQuery() or initForFullSubmit() before process()!");

   const short* pSourcePCMIt = pPCM;
   const short* pSourcePCMIt_end = pPCM + num_samples;

   if ( !pd.m_skipPassed )
   {
      // needs to skip data? (reminder: the query needs to skip QUERY_START_SECS (- half of the normalization window)
      if ( pd.m_skippedSoFar + num_samples > pd.m_toSkipSize )
      {
         pSourcePCMIt = pPCM + (pd.m_toSkipSize - pd.m_skippedSoFar);
         pd.m_skipPassed = true;
      }
      else
      {
         // need more data
         pd.m_skippedSoFar += num_samples;
         return false;
      }
   }

   pair<size_t, size_t> readData(0,0);
   pd.m_downsampleData.end_of_input = end_of_stream ? 1 : 0;

   //////////////////////////////////////////////////////////////////////////
   // PREBUFFER:
   if ( !pd.m_preBufferPassed )
   {
      // 1. downsample [norm + cb] frames to m_bufferSize - norm/2
      pd.m_floatInData.resize( (pSourcePCMIt_end - pSourcePCMIt) / pd.m_nchannels);
      src_short_to_float_and_mono_array( pSourcePCMIt, 
                                         &(pd.m_floatInData[0]), static_cast<int>(pSourcePCMIt_end - pSourcePCMIt), 
                                         pd.m_nchannels);

      pd.m_downsampleData.data_in = &(pd.m_floatInData[0]);
      pd.m_downsampleData.input_frames = static_cast<long>(pd.m_floatInData.size());

      pd.m_downsampleData.data_out = pd.m_pDownsampledCurrIt;
      pd.m_downsampleData.output_frames = static_cast<long>(pd.m_pEndDownsampledBuf - pd.m_pDownsampledCurrIt);

      int err = src_process(pd.m_pDownsampleState, &(pd.m_downsampleData));
      if ( err )
         throw std::runtime_error( src_strerror(err) );

      pd.m_pDownsampledCurrIt += pd.m_downsampleData.output_frames_gen;

      if ( pd.m_pDownsampledCurrIt != pd.m_pEndDownsampledBuf )
         return false; // NEED MORE DATA

      pSourcePCMIt += pd.m_downsampleData.input_frames_used * pd.m_nchannels;

      size_t pos = pd.m_downsampledProcessSize;
      size_t window_pos = pd.m_downsampledProcessSize - pd.m_normWindow.size() / 2;
      const size_t end_window_pos = window_pos + pd.m_normWindow.size();

      // 2. read [m_buffersize - norm/2...m_buffersize + norm/2] into normalize window
      for (; window_pos < end_window_pos ; ++window_pos)
         pd.m_normWindow.add(pd.m_pDownsampledPCM[window_pos] * pd.m_pDownsampledPCM[window_pos]);

      // 3. normalize [m_bufferSize..m_bufferSize+cb]
      for (; pos < pd.m_downsampledProcessSize + pd.m_compensateBufferSize; ++pos, ++window_pos)
      {
         pd.m_pDownsampledPCM[pos] /= getRMS(pd.m_normWindow);
         pd.m_normWindow.add(pd.m_pDownsampledPCM[window_pos] * pd.m_pDownsampledPCM[window_pos]);
      }

      pd.m_preBufferPassed = true;
   }

   //////////////////////////////////////////////////////////////////////////
   // PROCESS:

   bool found_enough_unique_keys = false;
   while (pd.m_toProcessKeys == 0 || pd.m_processedKeys < pd.m_toProcessKeys || !found_enough_unique_keys)
   {

      // 1. copy [m_bufferSize..m_bufferSize + cb + norm/2] to beginning
      if ( pd.m_pDownsampledCurrIt == pd.m_pEndDownsampledBuf )
      {
         memcpy( pd.m_pDownsampledPCM, pd.m_pDownsampledPCM + pd.m_downsampledProcessSize,
                (pd.m_compensateBufferSize + (pd.m_normWindow.size() / 2)) * sizeof(float));
         pd.m_pDownsampledCurrIt = pd.m_pDownsampledPCM + (pd.m_compensateBufferSize + (pd.m_normWindow.size() / 2));
      }

      // 2. read m_bufferSize frames to cb + norm/2
      pd.m_floatInData.resize( (pSourcePCMIt_end - pSourcePCMIt) / pd.m_nchannels);

      if ( pd.m_floatInData.empty() )
         return false;

      src_short_to_float_and_mono_array( pSourcePCMIt, 
                                         &(pd.m_floatInData[0]), static_cast<int>(pSourcePCMIt_end - pSourcePCMIt), 
                                         pd.m_nchannels);

      pd.m_downsampleData.data_in = &(pd.m_floatInData[0]);
      pd.m_downsampleData.input_frames = static_cast<long>(pd.m_floatInData.size());

      pd.m_downsampleData.data_out = pd.m_pDownsampledCurrIt;
      pd.m_downsampleData.output_frames = static_cast<long>(pd.m_pEndDownsampledBuf - pd.m_pDownsampledCurrIt);

      int err = src_process(pd.m_pDownsampleState, &(pd.m_downsampleData));
      if ( err )
         throw std::runtime_error( src_strerror(err) );

      pd.m_pDownsampledCurrIt += pd.m_downsampleData.output_frames_gen;

      if ( pd.m_pDownsampledCurrIt != pd.m_pEndDownsampledBuf && !end_of_stream )
         return false; // NEED MORE DATA

      //pSourcePCMIt += readData.second;
      pSourcePCMIt += pd.m_downsampleData.input_frames_used * pd.m_nchannels;

      // ********************************************************************

      // 3. normalize [cb..m_bufferSize+cb]
      size_t pos = static_cast<unsigned int>(pd.m_compensateBufferSize);
      size_t window_pos = static_cast<unsigned int>(pd.m_compensateBufferSize + (pd.m_normWindow.size() / 2));

      for(; pos < pd.m_downsampledProcessSize + pd.m_compensateBufferSize /* m_fullDownsampledBufferSize*/; ++pos, ++window_pos)
      {
         pd.m_pDownsampledPCM[pos] /= getRMS(pd.m_normWindow);
         pd.m_normWindow.add(pd.m_pDownsampledPCM[window_pos] * pd.m_pDownsampledPCM[window_pos]);
      }

      // 4. fft/process/whatevs [0...m_bufferSize+cb]
      pd.m_processedKeys += processKeys(pd.m_groupWindow, pos, pd);

      // we have too many keys, now we have to chop either one end or the other
      if (pd.m_toProcessKeys != 0 && pd.m_processedKeys > pd.m_toProcessKeys)
      {
         // set up window begin and end
         deque<GroupData>::iterator itBeg = pd.m_groupWindow.begin(), itEnd = pd.m_groupWindow.end();
         unsigned int offset_left, offset_right;

         found_enough_unique_keys = 
            fingerprint::findSignificantGroups( itBeg, itEnd, offset_left, offset_right, pd.m_toProcessKeys,
                                                pd.m_totalWindowKeys, pd.m_minUniqueKeys);

         // if we're happy with this set, snip the beginning and end of the grouped keys
         if (found_enough_unique_keys)
         {
            itBeg->count -= offset_left;
            if (offset_right > 0 && itEnd != pd.m_groupWindow.end())
            {
               itEnd->count = offset_right;
               ++itEnd;
            }
         }

         // chop the deque
         copy(itBeg, itEnd, pd.m_groupWindow.begin());
         pd.m_groupWindow.resize(itEnd - itBeg);            

         // recalc keys
         pd.m_processedKeys = 0;
         for (deque<GroupData>::const_iterator it = pd.m_groupWindow.begin(); it != pd.m_groupWindow.end(); ++it)
            pd.m_processedKeys += it->count;
      }

      if ( end_of_stream )
         break;

   } // while (totalKeys == 0 || keys < totalKeys || !found_enough_unique_keys)


   if (pd.m_toProcessKeys != 0 && pd.m_processedKeys < pd.m_toProcessKeys)
      throw std::runtime_error("Couldn't deliver the requested number of keys (it's the file too short?)");

   if ((pd.m_toProcessKeys != 0 && !found_enough_unique_keys) || 
       (pd.m_toProcessKeys == 0 && !enoughUniqueGoodGroups(pd.m_groupWindow.begin(), pd.m_groupWindow.end(), pd.m_minUniqueKeys)))
   {
      throw std::runtime_error("Not enough unique keys (it's the file too short?)");
   }

   // copy to a vector so that they can be returned as contiguous data
   pd.m_groups.resize(pd.m_groupWindow.size());
   copy(pd.m_groupWindow.begin(), pd.m_groupWindow.end(), pd.m_groups.begin());

   pd.m_groupsReady = true;
   pd.m_processType = PT_UNKNOWN;
   return true;
}

// -----------------------------------------------------------------------------

pair<const char*, size_t> FingerprintExtractor::getFingerprint()
{
   // easier read
   PimplData& pd = *m_pPimplData;

   if ( pd.m_groupsReady )
   {
#if __BIG_ENDIAN__
      pd.m_bigEndianGroups.resize(pd.m_groups.size());
      for ( size_t i = 0; i < pd.m_groups.size(); ++i )
      {
         pd.m_bigEndianGroups[i].key = reorderbits(pd.m_groups[i].key);
         pd.m_bigEndianGroups[i].count = reorderbits(pd.m_groups[i].count);
      }

      return make_pair(reinterpret_cast<const char*>(&pd.m_bigEndianGroups[0]), pd.m_bigEndianGroups.size() * sizeof(GroupData) );

#else
      return make_pair(reinterpret_cast<const char*>(&pd.m_groups[0]), pd.m_groups.size() * sizeof(GroupData) );
#endif
   }
   else
      return make_pair(reinterpret_cast<const char*>(0), 0); // here's where null_ptr would become useful!
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

float getRMS(const FloatingAverage<double>& signal)
{
   // we don't want to normalize by the real rms, because excessive clipping will occur
   float rms = sqrtf(static_cast<float>(signal.getAverage())) * 10.0F;

   if (rms < 0.1F)
      rms = 0.1F;
   else if (rms > 3.0F)
      rms = 3.0F;

   return rms;
}

// -----------------------------------------------------------------------------

unsigned int processKeys( deque<GroupData>& groups, size_t size, PimplData& pd )
{
   size_t read_size = min(size, pd.m_downsampledProcessSize + pd.m_compensateBufferSize);

   unsigned int numFrames = pd.m_pFFT->process(pd.m_pDownsampledPCM, read_size);

   if ( numFrames <= Filter::KEYWIDTH )
      return 0; // skip it when the number of frames is too small

   float** ppFrames = pd.m_pFFT->getFrames();

   integralImage(ppFrames, numFrames);
   computeBits(pd.m_partialBits, pd.m_filters, ppFrames, numFrames);
   fingerprint::keys2GroupData(pd.m_partialBits, groups, false);

   return static_cast<unsigned int>(pd.m_partialBits.size());

}

// -----------------------------------------------------------------------------

void integralImage(float** ppFrames, unsigned int nFrames)
{
   for (unsigned int y = 1; y < nFrames; y++) 
   {
      ppFrames[y][0] += ppFrames[y-1][0];
   }

   for (unsigned int x = 1; x < Filter::NBANDS; x++) 
   {
      ppFrames[0][x] += ppFrames[0][x-1];
   }

   for (unsigned int y = 1; y < nFrames; y++) 
   {
      for (unsigned int x = 1; x < Filter::NBANDS; x++) 
      {
         ppFrames[y][x] += static_cast<float>( static_cast<double>(ppFrames[y-1][x]) + 
                                               static_cast<double>(ppFrames[y][x-1]) - 
                                               static_cast<double>(ppFrames[y-1][x-1]) );
      }
   }
}

// ---------------------------------------------------------------------
//
/// Convert bands to bits, using the supplied filters
void computeBits( vector<unsigned int>& bits,
                  const vector<Filter>& f, 
                  float ** frames, unsigned int nframes ) 
{
   unsigned int first_time = Filter::KEYWIDTH / 2 + 1;
   unsigned int last_time = nframes - Filter::KEYWIDTH / 2;

   unsigned int numBits = last_time - first_time + 1;
   bits.resize(numBits);

   const unsigned int fSize = static_cast<unsigned int>(f.size());
   std::bitset<32> bt;
   double X = 0;

   for (unsigned int t2 = first_time; t2 <= last_time; ++t2) 
   {

      for (unsigned int i = 0; i < fSize; ++i) 
      {
         // we subtract 1 from t1 and b1 because we use integral images
      
         unsigned int t1 = (unsigned int) ((float) t2 - f[i].wt / 2.0 - 1);
         unsigned int t3 = (unsigned int) ((float) t2 + f[i].wt / 2.0 - 1);
         unsigned int b1 = f[i].first_band;
         unsigned int b2 = (unsigned int) round__((float) b1 + f[i].wb / 2.0) - 1;
         unsigned int b3 = b1 + f[i].wb - 1;
         --b1;

         unsigned int t_1q = (t1 + t2) / 2; // one quarter time 
         unsigned int t_3q = t_1q + (t3 - t1 + 1) / 2; // three quarter time
         unsigned int b_1q = (b1 + b2) / 2; // one quarter band
         unsigned int b_3q = b_1q + (b3 - b1) / 2; // three quarter band
         
         X = 0;
         
         // we should check from t1 > 0, but in practice, this doesn't happen
         // we subtract 1 from everything because this came from matlab where indices start from 1
         switch (f[i].filter_type) {
         case 1: { // total energy
            if (b1 > 0)
               X = static_cast<double>(frames[t3-1][b3-1]) - static_cast<double>(frames[t3-1][b1-1])
                 - static_cast<double>(frames[t1-1][b3-1]) + static_cast<double>(frames[t1-1][b1-1]);
            else
               X = static_cast<double>(frames[t3-1][b3-1]) - static_cast<double>(frames[t1-1][b3-1]);
            break;
         }
         case 2: { // energy difference over time
            if (b1 > 0)
               X = static_cast<double>(frames[t1-1][b1-1]) - 2*static_cast<double>(frames[t2-2][b1-1])
                 + static_cast<double>(frames[t3-1][b1-1]) - static_cast<double>(frames[t1-1][b3-1])
                 + 2*static_cast<double>(frames[t2-2][b3-1]) - static_cast<double>(frames[t3-1][b3-1]);
            else
               X = - static_cast<double>(frames[t1-1][b3-1]) + 2*static_cast<double>(frames[t2-2][b3-1])
                   - static_cast<double>(frames[t3-1][b3-1]);
            break;
         
         }
         case 3: { // energy difference over bands
            if (b1 > 0)
               X = static_cast<double>(frames[t1-1][b1-1]) - static_cast<double>(frames[t3-1][b1-1])
                 - 2*static_cast<double>(frames[t1-1][b2-1]) + 2*static_cast<double>(frames[t3-1][b2-1])
                 + static_cast<double>(frames[t1-1][b3-1]) - static_cast<double>(frames[t3-1][b3-1]);
            else
               X = - 2*static_cast<double>(frames[t1-1][b2-1]) + 2*static_cast<double>(frames[t3-1][b2-1])
                   + static_cast<double>(frames[t1-1][b3-1]) - static_cast<double>(frames[t3-1][b3-1]);
            break;   
         }
         case 4: {
            // energy difference over time and bands
            if (b1 > 0)
               X = static_cast<double>(frames[t1-1][b1-1]) - 2*static_cast<double>(frames[t2-2][b1-1])
                 + static_cast<double>(frames[t3-1][b1-1]) - 2*static_cast<double>(frames[t1-1][b2-1])
                 + 4*static_cast<double>(frames[t2-2][b2-1]) - 2*static_cast<double>(frames[t3-1][b2-1])
                 + static_cast<double>(frames[t1-1][b3-1]) - 2*static_cast<double>(frames[t2-2][b3-1])
                 + static_cast<double>(frames[t3-1][b3-1]);
            else
               X = - 2*static_cast<double>(frames[t1-1][b2-1]) + 4*static_cast<double>(frames[t2-2][b2-1])
                   - 2*static_cast<double>(frames[t3-1][b2-1]) + static_cast<double>(frames[t1-1][b3-1])
                   - 2*static_cast<double>(frames[t2-2][b3-1]) + static_cast<double>(frames[t3-1][b3-1]);
            break;   
         }
         case 5: { // time peak
            if (b1 > 0)
               X = - static_cast<double>(frames[t1-1][b1-1]) + 2*static_cast<double>(frames[t_1q-1][b1-1])
                   - 2*static_cast<double>(frames[t_3q-1][b1-1]) + static_cast<double>(frames[t3-1][b1-1])
                   + static_cast<double>(frames[t1-1][b3-1]) - 2*static_cast<double>(frames[t_1q-1][b3-1])
                   + 2*static_cast<double>(frames[t_3q-1][b3-1]) - static_cast<double>(frames[t3-1][b3-1]);
            else
               X = static_cast<double>(frames[t1-1][b3-1]) - 2*static_cast<double>(frames[t_1q-1][b3-1])
                 + 2*static_cast<double>(frames[t_3q-1][b3-1]) - static_cast<double>(frames[t3-1][b3-1]);
                  
            break;
         }
         case 6: { // band beak
            if (b1 > 0)
               X = - static_cast<double>(frames[t1-1][b1-1]) + static_cast<double>(frames[t3-1][b1-1])
                   + 2*static_cast<double>(frames[t1-1][b_1q-1]) - 2*static_cast<double>(frames[t3-1][b_1q-1])
                   - 2*static_cast<double>(frames[t1-1][b_3q-1]) + 2*static_cast<double>(frames[t3-1][b_3q-1])
                   + static_cast<double>(frames[t1-1][b3-1]) - static_cast<double>(frames[t3-1][b3-1]);
            else
               X = + 2*static_cast<double>(frames[t1-1][b_1q-1]) - 2*static_cast<double>(frames[t3-1][b_1q-1])
                   - 2*static_cast<double>(frames[t1-1][b_3q-1]) + 2*static_cast<double>(frames[t3-1][b_3q-1])
                   + static_cast<double>(frames[t1-1][b3-1]) - static_cast<double>(frames[t3-1][b3-1]);

            break;
         }
         }

         bt[i] = X > f[i].threshold;
      }

      bits[t2 - first_time] = bt.to_ulong();
   }
}

// -----------------------------------------------------------------------------

void src_short_to_float_and_mono_array( const short *in, float *out, int srclen, int nchannels )
{
   switch ( nchannels )
   {
   case 1:
      src_short_to_float_array(in, out, srclen);
      break;
   case 2:
      {
         // this can be optimized
         int j = 0;
         const double div = numeric_limits<short>::max() * nchannels;
         for ( int i = 0; i < srclen; i += 2, ++j )
         {
            out[j] = static_cast<float>( static_cast<double>(static_cast<int>(in[i]) + static_cast<int>(in[i+1])) / div );
         }
      }
      break;

   default:
      throw( std::runtime_error("Unsupported number of channels!") );
   }

}

// -----------------------------------------------------------------------------

} // end of namespace

// -----------------------------------------------------------------------------
