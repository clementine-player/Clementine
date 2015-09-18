/* This file is part of Clementine.
   Copyright 2013, David Sansome <me@davidsansome.com>

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

#ifndef TAGREADER_H
#define TAGREADER_H

#include <QByteArray>

#include <taglib/xiphcomment.h>

#include "config.h"
#include "tagreadermessages.pb.h"

class QNetworkAccessManager;
class QString;
class QTextCodec;
class QUrl;

namespace TagLib {
class FileRef;
class String;

namespace ID3v2 {
class Tag;
class PopularimeterFrame;
}
}

class FileRefFactory;

/**
 * This class holds all useful methods to read and write tags from/to files.
 * You should not use it directly in the main process but rather use a
 * TagReaderWorker process (using TagReaderClient)
 */
class TagReader {
 public:
  TagReader();

  void ReadFile(const QString& filename,
                pb::tagreader::SongMetadata* song) const;
  bool SaveFile(const QString& filename,
                const pb::tagreader::SongMetadata& song) const;
  // Returns false if something went wrong; returns true otherwise (might
  // returns true if the file exists but nothing has been written inside because
  // statistics tag format is not supported for this kind of file)
  bool SaveSongStatisticsToFile(const QString& filename,
                                const pb::tagreader::SongMetadata& song) const;
  bool SaveSongRatingToFile(const QString& filename,
                            const pb::tagreader::SongMetadata& song) const;

  bool IsMediaFile(const QString& filename) const;
  QByteArray LoadEmbeddedArt(const QString& filename) const;

#ifdef HAVE_GOOGLE_DRIVE
  bool ReadCloudFile(const QUrl& download_url, const QString& title, int size,
                     const QString& mime_type, const QString& access_token,
                     pb::tagreader::SongMetadata* song) const;
#endif  // HAVE_GOOGLE_DRIVE

  static void Decode(const TagLib::String& tag, const QTextCodec* codec,
                     std::string* output);
  static void Decode(const QString& tag, const QTextCodec* codec,
                     std::string* output);

  void ParseFMPSFrame(const QString& name, const QString& value,
                      pb::tagreader::SongMetadata* song) const;
  void ParseOggTag(const TagLib::Ogg::FieldListMap& map,
                   const QTextCodec* codec, QString* disc, QString* compilation,
                   pb::tagreader::SongMetadata* song) const;
  void SetVorbisComments(TagLib::Ogg::XiphComment* vorbis_comments,
                         const pb::tagreader::SongMetadata& song) const;
  void SetFMPSStatisticsVorbisComments(
      TagLib::Ogg::XiphComment* vorbis_comments,
      const pb::tagreader::SongMetadata& song) const;
  void SetFMPSRatingVorbisComments(
      TagLib::Ogg::XiphComment* vorbis_comments,
      const pb::tagreader::SongMetadata& song) const;

  pb::tagreader::SongMetadata_Type GuessFileType(
      TagLib::FileRef* fileref) const;

  void SetUserTextFrame(const QString& description, const QString& value,
                        TagLib::ID3v2::Tag* tag) const;
  void SetUserTextFrame(const std::string& description,
                        const std::string& value,
                        TagLib::ID3v2::Tag* tag) const;

  void SetTextFrame(const char* id, const QString& value,
                    TagLib::ID3v2::Tag* tag) const;
  void SetTextFrame(const char* id, const std::string& value,
                    TagLib::ID3v2::Tag* tag) const;
  void SetUnsyncLyricsFrame(const std::string& value,
                            TagLib::ID3v2::Tag* tag) const;

 private:
  static const char* kMP4_FMPS_Rating_ID;
  static const char* kMP4_FMPS_Playcount_ID;
  static const char* kMP4_FMPS_Score_ID;
  // Returns a float in [0.0..1.0] corresponding to the rating range we use in
  // Clementine
  static float ConvertPOPMRating(const int POPM_rating);
  // Reciprocal
  static int ConvertToPOPMRating(const float rating);
  static TagLib::ID3v2::PopularimeterFrame* GetPOPMFrameFromTag(
      TagLib::ID3v2::Tag* tag);

  FileRefFactory* factory_;
  QNetworkAccessManager* network_;

  const std::string kEmbeddedCover;
};

#endif  // TAGREADER_H
