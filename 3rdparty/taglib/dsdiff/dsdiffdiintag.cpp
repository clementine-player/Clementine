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

#include "dsdiffdiintag.h"
#include "tstringlist.h"
#include "tpropertymap.h"

using namespace TagLib;
using namespace DSDIFF::DIIN;

class DSDIFF::DIIN::Tag::TagPrivate
{
public:
  TagPrivate()
  {
  }

  String title;
  String artist;
};

DSDIFF::DIIN::Tag::Tag() : TagLib::Tag()
{
  d = new TagPrivate;
}

DSDIFF::DIIN::Tag::~Tag()
{
  delete d;
}

String DSDIFF::DIIN::Tag::title() const
{
  return d->title;
}

String DSDIFF::DIIN::Tag::artist() const
{
  return d->artist;
}

String DSDIFF::DIIN::Tag::album() const
{
  return String();
}

String DSDIFF::DIIN::Tag::comment() const
{
  return String();
}

String DSDIFF::DIIN::Tag::genre() const
{
  return String();
}

unsigned int DSDIFF::DIIN::Tag::year() const
{
  return 0;
}

unsigned int DSDIFF::DIIN::Tag::track() const
{
  return 0;
}

void DSDIFF::DIIN::Tag::setTitle(const String &title)
{
  if(title.isNull() || title.isEmpty())
    d->title = String();
  else
    d->title = title;
}

void DSDIFF::DIIN::Tag::setArtist(const String &artist)
{
  if(artist.isNull() || artist.isEmpty())
    d->artist = String();
  else
    d->artist = artist;
}

void DSDIFF::DIIN::Tag::setAlbum(const String &)
{
}

void DSDIFF::DIIN::Tag::setComment(const String &)
{
}

void DSDIFF::DIIN::Tag::setGenre(const String &)
{
}

void DSDIFF::DIIN::Tag::setYear(unsigned int)
{
}

void DSDIFF::DIIN::Tag::setTrack(unsigned int)
{
}

PropertyMap DSDIFF::DIIN::Tag::properties() const
{
  PropertyMap properties;
  properties["TITLE"] = d->title;
  properties["ARTIST"] = d->artist;
  return properties;
}

PropertyMap DSDIFF::DIIN::Tag::setProperties(const PropertyMap &origProps)
{
  PropertyMap properties(origProps);
  properties.removeEmpty();
  StringList oneValueSet;

  if(properties.contains("TITLE")) {
    d->title = properties["TITLE"].front();
    oneValueSet.append("TITLE");
  } else
    d->title = String();

  if(properties.contains("ARTIST")) {
    d->artist = properties["ARTIST"].front();
    oneValueSet.append("ARTIST");
  } else
    d->artist = String();

  // for each tag that has been set above, remove the first entry in the corresponding
  // value list. The others will be returned as unsupported by this format.
  for(StringList::Iterator it = oneValueSet.begin(); it != oneValueSet.end(); ++it) {
    if(properties[*it].size() == 1)
      properties.erase(*it);
    else
      properties[*it].erase(properties[*it].begin());
  }

  return properties;
}

