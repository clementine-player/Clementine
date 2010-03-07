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
