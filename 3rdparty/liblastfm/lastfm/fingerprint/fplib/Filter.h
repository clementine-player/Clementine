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
#ifndef __FILTER_H
#define __FILTER_H

namespace fingerprint
{

struct Filter 
{
    /// Constructs a new filter with id.
    Filter(unsigned int id, float threshold, float weight);
   
   unsigned int id; //< filter id
    unsigned int wt; //< time width
    unsigned int first_band; //< first band
    unsigned int wb; //< band width
    unsigned int filter_type; //< filter type
    
    float threshold; //< threshold for filter
    float weight; //< filter weight

   // number of frames in time
   static const unsigned int KEYWIDTH = 100;
   // number of bands to divide the signal (log step)
   static const unsigned int NBANDS = 33;
};

}

#endif // __FILTER_H
