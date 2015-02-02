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
#ifndef __FINGERPRINT_HELPER_FUNCTIONS_H
#define __FINGERPRINT_HELPER_FUNCTIONS_H

#include <vector>
#include <deque>
#include <set>

namespace fingerprint
{

// -----------------------------------------------------------------------------

static const size_t FINGERPRINT_LIB_VERSION = 1;
static const float  QUERY_START_SECS = 20;
static const float  QUERY_SIZE_SECS = 14;
static const float  UPDATE_SIZE_SECS = 10;
//FFT needs also a buffer that depends on the input freq. 3 secs should be enough up to 48Khz 
static const float  GUARD_SIZE_SECS = 3; 
static const float  NORMALIZATION_SKIP_SECS = 2.5;
static const int    MIN_UNIQUE_KEYS = 75;
static const unsigned int MAX_GOOD_GROUP_SIZE = 200;
static const int    SHA_SIZE = 32;

/////////////////////////////////////////////////////
// For FFT. DO NOT TOUCH THEM!
// number of samples in a frame
static const int FRAMESIZE = 2048;
static const int OVERLAP = 32;
static const int OVERLAPSAMPLES = (FRAMESIZE/OVERLAP); // 64

// down-sampled frequency
static const int DFREQ = 5512;
static const float FDFREQ = 5512.5f;

// -----------------------------------------------------------------------------

struct GroupData
{
   unsigned int key;    // the key (or local descriptor)
   unsigned int count;  // the number of frames sharing this key
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

inline 
unsigned int getTotalKeys( 
   int mSecs )
{
   return static_cast<unsigned int>((static_cast<double>(mSecs) / (1000.0 * OVERLAPSAMPLES) ) * DFREQ ) + 1;
}

// -----------------------------------------------------------------------------

template <typename GroupDataIt>
void simpleSkip( 
   GroupDataIt& begIt, const GroupDataIt& endIt,
   unsigned int numSkipKeys )
{
   if ( numSkipKeys <= 0 )
      return;

   unsigned int nKeys;
   for ( nKeys = 0; nKeys < numSkipKeys && begIt != endIt; ++begIt )
      nKeys += begIt->count;

   // clear crop at the end
   if ( nKeys > numSkipKeys )
   {
      --begIt;
      begIt->count = nKeys - numSkipKeys;
   }

}

// -----------------------------------------------------------------------------

template <typename TGroupData>
void cutGroups( 
   std::vector<TGroupData>& groups, 
   const unsigned int startMS, 
   const unsigned int lengthMS )
{
   typename  std::vector<TGroupData>::iterator itBeg = groups.begin(), itEnd = groups.begin();

   unsigned int keys_begin, keys_end;

   for (keys_begin = getTotalKeys(startMS);
                  itBeg != groups.end() && keys_begin > itBeg->count; ++itBeg)
      keys_begin -= itBeg->count;

   for (keys_end = getTotalKeys(startMS + lengthMS);
        itEnd != groups.end() && keys_end > itEnd->count; ++itEnd)
      keys_end -= itEnd->count;

   if (itBeg == groups.end())  // in the umpossible scenario that you try to cut past the size of the groups
   {
      groups.clear();
      return;
   }

   itBeg->count -= keys_begin;
   if (keys_end > 0 && itEnd != groups.end())
   {
      itEnd->count = keys_end;
      ++itEnd;
   }

   copy(itBeg, itEnd, groups.begin());
   groups.resize(itEnd - itBeg);

   keys_begin = getTotalKeys(lengthMS);
   for (typename std::vector<TGroupData>::iterator it = groups.begin(); it != groups.end(); ++it)
      keys_begin -= it->count;
}

// -------------------------------------------------------------------------

template <typename TGroupData>
void keys2GroupData( 
   const std::vector<unsigned int>& keys, // in
   std::vector<TGroupData>& groupData,
   bool clearDst = true ) // out
{
   if (clearDst)
      groupData.clear();

   if (keys.empty())
      return;

   TGroupData tmpGroup;
   std::vector<unsigned int>::const_iterator it = keys.begin();

   if ( !groupData.empty() )
   {
      // get the last group 
      tmpGroup = groupData.back();
      groupData.pop_back();
   }
   else
   {
      // new group!
      tmpGroup.key = *it;
      tmpGroup.count = 1;
      ++it; // move to the next key
   }

   for (; it != keys.end(); ++it)
   {
      if ( *it != tmpGroup.key )
      {
         // new group ready!
         groupData.push_back( tmpGroup );
         tmpGroup.key = *it;
         tmpGroup.count = 0;
      }

      ++tmpGroup.count;
   }

   // last group
   groupData.push_back( tmpGroup );
}

// -------------------------------------------------------------------------

template <typename TGroupData>
void keys2GroupData( 
   const std::vector<unsigned int>& keys, // in
   std::deque<TGroupData>& groupData,
   bool clearDst = true ) // out
{
   if (clearDst)
      groupData.clear();

   if (keys.empty())
      return;

   TGroupData tmpGroup;
   std::vector<unsigned int>::const_iterator it = keys.begin();

   if ( !groupData.empty() )
   {
      // get the last group 
      tmpGroup = groupData.back();
      groupData.pop_back();
   }
   else
   {
      // new group!
      tmpGroup.key = *it;
      tmpGroup.count = 1;
      ++it; // move to the next key
   }

   for (; it != keys.end(); ++it)
   {
      if ( *it != tmpGroup.key )
      {
         // new group ready!
         groupData.push_back( tmpGroup );
         tmpGroup.key = *it;
         tmpGroup.count = 0;
      }

      ++tmpGroup.count;
   }

   // last group
   groupData.push_back( tmpGroup );
}

// -------------------------------------------------------------------------

template <typename TGroupData>
inline 
void groupData2Keys( 
   const std::vector<TGroupData>& groupData, // in
   std::vector<unsigned int>& keys ) // out
{
   keys.clear();

   typename std::vector<TGroupData>::const_iterator it;
   
   for (it = groupData.begin(); it != groupData.end(); ++it)
   {
      for (unsigned int j = 0; j < it->count; ++j)
         keys.push_back(it->key);
   }
}

// -------------------------------------------------------------------------

template <typename GroupDataIt>
bool findSignificantGroups(
   GroupDataIt& beg, GroupDataIt& end, unsigned int& offset_left, unsigned int& offset_right,
   unsigned int windowKeySize, unsigned int subWindowKeySize, unsigned int minUniqueKeys)
{
   GroupDataIt itBeg = beg, itEnd = beg, itWindowBeg = beg, itWindowEnd = beg;

   offset_left = 0;
   unsigned int window_offset_left;
   unsigned int window_offset_right;

   // this amounts to around a 500 ms hop for, say, a 20 second sub-window
   unsigned int key_hop_size = subWindowKeySize / 40;  

   // trail out itEnd
   for (offset_right = windowKeySize; itEnd != end && offset_right > itEnd->count; ++itEnd)
      offset_right -= itEnd->count;

   // dang man, we don't even have enough groups to span the window size
   if (itEnd == end && offset_right > 0)
      return false;

   // 0 window size means just scan the whole range
   if (windowKeySize == 0)
      itEnd = end;

   // trail out itWindowBeg
   for (window_offset_left = (windowKeySize - subWindowKeySize) / 2;
        window_offset_left > itWindowBeg->count; ++itWindowBeg)
      window_offset_left -= itWindowBeg->count;

   // trail out itWindowEnd
   for (window_offset_right = (windowKeySize + subWindowKeySize) / 2;
      window_offset_right > itWindowEnd->count; ++itWindowEnd)
      window_offset_right -= itWindowEnd->count;

   while (itEnd != end)
   {
      if (enoughUniqueGoodGroups(itWindowBeg, itWindowEnd, minUniqueKeys))
      {
         beg = itBeg;
         end = itEnd;
         return true;
      }

      // okay, jump key_hop_size on end iterator
      for (offset_right += key_hop_size; itEnd != end && offset_right > itEnd->count; ++itEnd)
         offset_right -= itEnd->count;

      // if we didn't hop the full hop size, modify the hop size to only hop as far as we hopped
      if (itEnd == end)
         key_hop_size -= offset_right;

      for (offset_left += key_hop_size; offset_left > itBeg->count; ++itBeg)
         offset_left -= itBeg->count;
      for (window_offset_right += key_hop_size; window_offset_right > itWindowEnd->count; ++itWindowEnd)
         window_offset_right -= itWindowEnd->count;
      for (window_offset_left += key_hop_size; window_offset_left > itWindowBeg->count; ++itWindowBeg)
         window_offset_left -= itWindowBeg->count;
   }

   beg = itBeg;
   end = itEnd;

   return enoughUniqueGoodGroups(itWindowBeg, itWindowEnd, minUniqueKeys);
}

// -----------------------------------------------------------------------------

template <typename TGroupData>
bool
reduceGroups( 
  std::vector<TGroupData>& groups, unsigned int startKeySize,
  unsigned int windowKeySize, unsigned int subWindowKeySize, unsigned int minUniqueKeys )
{
   unsigned int offset_left = 0;
   unsigned int offset_right = 0;

   typename std::vector<TGroupData>::iterator begIt = groups.begin();
   typename std::vector<TGroupData>::iterator endIt = groups.end();

   simpleSkip(begIt, endIt, startKeySize);
   bool result = findSignificantGroups( begIt, endIt, 
      offset_left, offset_right, 
      windowKeySize, subWindowKeySize, minUniqueKeys );

   if ( !result )
   {
      groups.clear();
      return false;
   }

   begIt->count -= offset_left;
   if (offset_right > 0 && endIt != groups.end())
   {
      endIt->count = offset_right;
      ++endIt;
   }

   std::vector<TGroupData> resGrups(begIt, endIt);
   groups.swap(resGrups);

   return true;
}


// -------------------------------------------------------------------------

template <typename GroupDataIt>
inline bool enoughUniqueGoodGroups(
   const GroupDataIt& beg, 
   const GroupDataIt& end, 
   unsigned int minUniqueKeys)
{
   std::set<unsigned int> groupKeys;

   for (GroupDataIt it = beg; it != end && static_cast<unsigned int>(groupKeys.size()) < minUniqueKeys; ++it)
   {
      if (it->count > MAX_GOOD_GROUP_SIZE)
         return false;

      groupKeys.insert(it->key);
   }
   
   return static_cast<unsigned int>(groupKeys.size()) >= minUniqueKeys;
}

// -----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Used by the fingerprint keys operation

// minimum and maximum frequency to consider
#define MINFREQ 300
#define MAXFREQ 2000

// amount of time in a frame
#define FRAME_TLEN ((float) FRAMESIZE / (float) DFREQ)
#define MINCOEF (FRAME_TLEN * MINFREQ)

#define round__(x)  ((int)(x + .5))

struct RawFilter
{
   unsigned int ftid;
   float thresh;
    float weight;
};

const RawFilter rFilters[] = {
   { 26752, -4.37515e-07f, 0.260836f }, // filterID, threshold, alpha (weight)
   { 23871, -2.44615e-05f, 0.263986f },
   { 26777, -3.69244e-08f, 0.267763f },
   { 4635,  -1.13672e-05f, 0.269428f },
   { 2937,   5.28804e-09f, 0.271896f },
   { 27405, -0.000126494f, 0.272362f },
   { 10782,  4.27478e-08f, 0.272609f },
   { 21033, -6.7912e-07f,  0.276099f },
   { 27117,  8.07178e-06f, 0.277762f },
   { 27072,  2.46044e-05f, 0.27883f  },
   { 24228,  4.11255e-07f, 0.281743f },
   { 23838,  0.000228396f, 0.284479f },
   { 17165, -1.19495e-07f, 0.286304f },
   { 25263,  0.000398279f, 0.287066f },
   { 20721,  7.15095e-07f, 0.288913f },
   { 8502,  -2.78361e-07f, 0.290424f },
   { 17175, -1.08429e-08f, 0.292219f },
   { 17811, -3.29527e-08f, 0.292554f },
   { 27495, -4.47575e-07f, 0.290119f },
   { 23538, -3.04273e-09f, 0.294539f },
   { 8205,   4.02691e-07f, 0.293525f },
   { 12177,  1.16873e-06f, 0.293832f },
   { 27051, -0.000902544f, 0.296453f },
   { 27111, -2.38425e-05f, 0.297428f },
   { 21779, -1.0669e-07f,  0.297302f },
   { 14817, -9.52849e-09f, 0.299f    },
   { 27087,  1.22163e-05f, 0.296502f },
   { 27081, -2.8758e-09f,  0.300112f },
   { 20394,  1.28237e-06f, 0.298693f },
   { 28209,  0.000624447f, 0.29812f  },
   { 23533, -2.19406e-06f, 0.299773f },
   { 23865, -1.28037e-08f, 0.300777f } // this is iteration 1
};

// -----------------------------------------------------------------------------

}

// -----------------------------------------------------------------------------

#endif // __FINGERPRINT_HELPER_FUNCTIONS_H

