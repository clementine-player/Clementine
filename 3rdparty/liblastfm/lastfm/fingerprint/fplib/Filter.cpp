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
#include <algorithm> // for max
#include <vector>

#include "Filter.h"
#include "fp_helper_fun.h"

using namespace std;

namespace fingerprint
{

Filter::Filter(unsigned int id, float threshold, float weight) 
: id(id), threshold(threshold), weight(weight)
{
    float time_rate = 1.5;
        
    unsigned int t = 1;
    vector<unsigned int> time_lengths;

    while (t < KEYWIDTH) 
   {
        time_lengths.push_back(t);
      t = max( static_cast<unsigned int>( round__(time_rate*t) ) +
               static_cast<unsigned int>( round__(time_rate*t) % 2),
               t+1 );
    }

    unsigned int filter_count = 0;

    for (wt = 1; wt <= time_lengths.size(); wt++) 
   {
        for (wb = 1; wb <= NBANDS; wb++) 
      {
            for (first_band = 1; first_band <= NBANDS - wb + 1;
                 first_band++) 
         {
                unsigned int time = time_lengths[wt-1];
                filter_count++;

                if (filter_count == id) 
            {
                    wt = time_lengths[wt-1];
                    filter_type = 1;
                    return;
                }

                if (time > 1) 
            {
                    filter_count++;
                    if (filter_count == id) 
               {
                        wt = time_lengths[wt-1];
                        filter_type = 2;
                        return;
                    }
                }
                    
                if (wb > 1) 
            {
                    filter_count++;
                    if (filter_count == id) 
               {
                        wt = time_lengths[wt-1];
                        filter_type = 3;
                        return;
                    }
                }

                if (time > 1 && wb > 1) 
            {
                    filter_count++;
                    if (filter_count == id) 
               {
                        wt = time_lengths[wt-1];
                        filter_type = 4;
                        return;
                    }
                }
                
                if (time > 3) 
            {
                    filter_count++;
                    if (filter_count == id) 
               {
                        wt = time_lengths[wt-1];
                        filter_type = 5;
                        return;
                    }
                }
                
                if (wb > 3) 
            {
                    filter_count++;
                    if (filter_count == id) 
               {
                        wt = time_lengths[wt-1];
                        filter_type = 6;
                        return;
                    }
                }

            } // for first_band
        } // for wb
    } // for wt
}

} // end of namespace fingerprint

// -----------------------------------------------------------------------------
