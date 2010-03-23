/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MOCK_TAGLIB_H
#define MOCK_TAGLIB_H

#include "gmock/gmock.h"

#include <QMap>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#include <boost/scoped_ptr.hpp>

#include "song.h"

class MockTag : public TagLib::Tag {
 public:
  MOCK_CONST_METHOD0(title, TagLib::String());
  MOCK_CONST_METHOD0(artist, TagLib::String());
  MOCK_CONST_METHOD0(album, TagLib::String());
  MOCK_CONST_METHOD0(comment, TagLib::String());
  MOCK_CONST_METHOD0(genre, TagLib::String());
  MOCK_METHOD1(setTitle, void(const TagLib::String&));
  MOCK_METHOD1(setArtist, void(const TagLib::String&));
  MOCK_METHOD1(setAlbum, void(const TagLib::String&));
  MOCK_METHOD1(setComment, void(const TagLib::String&));
  MOCK_METHOD1(setGenre, void(const TagLib::String&));

  MOCK_CONST_METHOD0(year, uint());
  MOCK_CONST_METHOD0(track, uint());
  MOCK_METHOD1(setYear, void(uint));
  MOCK_METHOD1(setTrack, void(uint));
};


class MockFile : public TagLib::File {
 public:
  MockFile(TagLib::Tag* tag, const QString& filename);

  virtual TagLib::Tag* tag() const;

  virtual TagLib::AudioProperties* audioProperties() const;

  virtual bool save();

 protected:
  boost::scoped_ptr<TagLib::Tag> tag_;
};

class MockFileRefFactory : public FileRefFactory {
 public:
  void ExpectCall(const QString& filename,
                  const QString& title,
                  const QString& artist,
                  const QString& album);
 protected:
  virtual TagLib::FileRef* GetFileRef(const QString& filename);

  QMap<QString, MockTag*> tags_;
};

#endif  // MOCK_TAGLIB_H
