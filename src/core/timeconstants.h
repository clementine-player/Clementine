/* This file is part of Clementine.
   Copyright 2011-2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Note: this file is licensed under the Apache License instead of GPL because
// it is used by the Spotify blob which links against libspotify and is not GPL
// compatible.

#ifndef CORE_TIMECONSTANTS_H_
#define CORE_TIMECONSTANTS_H_

#include <QtGlobal>

// Use these to convert between time units
const qint64 kMsecPerSec = 1000ll;
const qint64 kUsecPerMsec = 1000ll;
const qint64 kUsecPerSec = 1000000ll;
const qint64 kNsecPerUsec = 1000ll;
const qint64 kNsecPerMsec = 1000000ll;
const qint64 kNsecPerSec = 1000000000ll;

const qint64 kSecsPerDay = 24 * 60 * 60;

#endif  // CORE_TIMECONSTANTS_H_
