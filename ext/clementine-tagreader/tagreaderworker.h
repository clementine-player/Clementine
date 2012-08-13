/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef TAGREADERWORKER_H
#define TAGREADERWORKER_H

#include "config.h"
#include "tagreadermessages.pb.h"
#include "core/messagehandler.h"

#include <taglib/xiphcomment.h>

#include <QUrl>

class QNetworkAccessManager;


namespace TagLib {
  class FileRef;
  class String;

  namespace ID3v2 {
    class Tag;
  }
}

class FileRefFactory;

class TagReaderWorker : public AbstractMessageHandler<pb::tagreader::Message> {
public:
  TagReaderWorker(QIODevice* socket, QObject* parent = NULL);

protected:
  void MessageArrived(const pb::tagreader::Message& message);
  void DeviceClosed();

private:
  void ReadFile(const QString& filename, pb::tagreader::SongMetadata* song) const;
  bool SaveFile(const QString& filename, const pb::tagreader::SongMetadata& song) const;
  bool IsMediaFile(const QString& filename) const;
  QByteArray LoadEmbeddedArt(const QString& filename) const;

  #ifdef HAVE_GOOGLE_DRIVE
  bool ReadGoogleDrive(const QUrl& download_url,
                       const QString& title,
                       int size,
                       const QString& mime_type,
                       const QString& access_token,
                       pb::tagreader::SongMetadata* song) const;
  #endif // HAVE_GOOGLE_DRIVE

  static void Decode(const TagLib::String& tag, const QTextCodec* codec,
                     std::string* output);
  static void Decode(const QString& tag, const QTextCodec* codec,
                     std::string* output);

  void ParseFMPSFrame(const QString& name, const QString& value,
                      pb::tagreader::SongMetadata* song) const;
  void ParseOggTag(const TagLib::Ogg::FieldListMap& map,
                   const QTextCodec* codec,
                   QString* disc, QString* compilation,
                   pb::tagreader::SongMetadata* song) const;
  pb::tagreader::SongMetadata_Type GuessFileType(TagLib::FileRef* fileref) const;

  void SetTextFrame(const char* id, const QString& value,
                    TagLib::ID3v2::Tag* tag) const;
  void SetTextFrame(const char* id, const std::string& value,
                    TagLib::ID3v2::Tag* tag) const;

private:
  FileRefFactory* factory_;
  QNetworkAccessManager* network_;

  const std::string kEmbeddedCover;
};

#endif // TAGREADERWORKER_H
