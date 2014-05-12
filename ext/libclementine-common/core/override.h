/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef OVERRIDE_H
#define OVERRIDE_H

// Defines the OVERRIDE macro as C++11's override control keyword if
// it is available.

#ifndef __has_extension
#define __has_extension(x) 0
#endif

#if __has_extension(cxx_override_control)  // Clang feature checking macro.
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#endif  // OVERRIDE_H
