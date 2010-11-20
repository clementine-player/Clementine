/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "mock_taglib.h"

#include <QFile>

using ::testing::Return;

MockFile::MockFile(TagLib::Tag* tag, const QString& filename)
    : TagLib::File(QFile::encodeName(filename).constData()),
      tag_(tag) {
}

TagLib::Tag* MockFile::tag() const {
  return tag_.get();
}

TagLib::AudioProperties* MockFile::audioProperties() const {
  return NULL;
}

bool MockFile::save() {
  return true;
}

void MockFileRefFactory::ExpectCall(const QString& filename,
                                    const QString& title,
                                    const QString& artist,
                                    const QString& album) {
  MockTag* tag = new MockTag;
  EXPECT_CALL(*tag, title()).WillRepeatedly(Return(title.toStdString()));
  EXPECT_CALL(*tag, artist()).WillRepeatedly(Return(artist.toStdString()));
  EXPECT_CALL(*tag, album()).WillRepeatedly(Return(album.toStdString()));
  tags_[filename] = tag;
}

TagLib::FileRef* MockFileRefFactory::GetFileRef(const QString& filename) {
  MockTag* tag = tags_.take(filename);
  MockFile* file = new MockFile(tag, filename);
  TagLib::FileRef* fileref = new TagLib::FileRef(file);
  return fileref;
}
