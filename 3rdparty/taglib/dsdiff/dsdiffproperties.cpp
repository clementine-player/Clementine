/***************************************************************************
 copyright            : (C) 2016 by Damien Plisson, Audirvana
 email                : damien78@audirvana.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <tstring.h>
#include <tdebug.h>

#include "dsdiffproperties.h"

using namespace TagLib;

class DSDIFF::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
  length(0),
  bitrate(0),
  sampleRate(0),
  channels(0),
  sampleWidth(0),
  sampleCount(0)
  {
  }

  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int sampleWidth;
  unsigned long long sampleCount;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSDIFF::Properties::Properties(const unsigned int sampleRate,
                               const unsigned short channels,
                               const unsigned long long samplesCount,
                               const int bitrate,
                               ReadStyle style) : AudioProperties(style)
{
  d = new PropertiesPrivate;

  d->channels    = channels;
  d->sampleCount = samplesCount;
  d->sampleWidth = 1;
  d->sampleRate  = sampleRate;
  d->bitrate     = bitrate;
  d->length      = d->sampleRate > 0
    ? static_cast<int>((d->sampleCount * 1000.0) / d->sampleRate + 0.5)
    : 0;
}

DSDIFF::Properties::~Properties()
{
  delete d;
}

int DSDIFF::Properties::length() const
{
  return lengthInSeconds();
}

int DSDIFF::Properties::lengthInSeconds() const
{
  return d->length / 1000;
}

int DSDIFF::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int DSDIFF::Properties::bitrate() const
{
  return d->bitrate;
}

int DSDIFF::Properties::sampleRate() const
{
  return d->sampleRate;
}

int DSDIFF::Properties::channels() const
{
  return d->channels;
}

int DSDIFF::Properties::bitsPerSample() const
{
  return d->sampleWidth;
}

long long DSDIFF::Properties::sampleCount() const
{
  return d->sampleCount;
}

