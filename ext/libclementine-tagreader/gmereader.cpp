#include "core/logging.h"
#include "core/timeconstants.h"
#include "gmereader.h"
#include "tagreader.h"

#include <apefile.h>
#include <tag.h>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtEndian>

bool GME::IsSupportedFormat(const QFileInfo& file_info) {
  return file_info.completeSuffix().endsWith("spc");
}

void GME::ReadFile(const QFileInfo& file_info,
                   pb::tagreader::SongMetadata* song_info) {
  if (file_info.completeSuffix().endsWith("spc"))
    GME::SPC::Read(file_info, song_info);
}

void GME::SPC::Read(const QFileInfo& file_info,
                    pb::tagreader::SongMetadata* song_info) {
  QFile file(file_info.filePath());
  if (!file.open(QIODevice::ReadOnly)) return;

  qLog(Debug) << "Reading SPC from file" << file_info.fileName();

  // Check for header -- more reliable than file name alone.
  if (!file.read(33).startsWith(QString("SNES-SPC700").toAscii())) return;

  /*
   * First order of business -- get any tag values that exist within the core
   * file information. These only allow for a certain number of bytes
   * per field, so they will likely be overwritten either by the id666 standard
   * or the APETAG format (as used by other players, such as foobar and winamp)
   *
   * Make sure to check id6 documentation before changing the read values!
   */

  file.seek(HAS_ID6_OFFSET);
  bool has_id6 = (file.read(1)[0] == (char)xID6_STATUS::ON);

  file.seek(SONG_TITLE_OFFSET);
  song_info->set_title(QString::fromAscii(file.read(32)).toStdString());

  file.seek(GAME_TITLE_OFFSET);
  song_info->set_album(QString::fromAscii(file.read(32)).toStdString());

  file.seek(ARTIST_OFFSET);
  song_info->set_artist(QString::fromAscii(file.read(32)).toStdString());

  file.seek(INTRO_LENGTH_OFFSET);
  QByteArray length_bytes = file.read(INTRO_LENGTH_SIZE);
  quint64 length_in_sec = 0;
  if (length_bytes.size() >= INTRO_LENGTH_SIZE) {
    length_in_sec = ConvertSPCStringToNum(length_bytes);
    qLog(Debug) << length_in_sec << "------ LENGTH";

    if (!length_in_sec || length_in_sec >= 0x1FFF) {
      // This means that parsing the length as a string failed, so get value LE.
      length_in_sec =
          length_bytes[0] | (length_bytes[1] << 8) | (length_bytes[2] << 16);
    }

    if (length_in_sec < 0x1FFF) {
      song_info->set_length_nanosec(length_in_sec * kNsecPerSec);
    }
  }

  file.seek(FADE_LENGTH_OFFSET);
  QByteArray fade_bytes = file.read(FADE_LENGTH_SIZE);
  if (fade_bytes.size() >= FADE_LENGTH_SIZE) {
    quint64 fade_length_in_ms = ConvertSPCStringToNum(fade_bytes);
    qLog(Debug) << fade_length_in_ms << "------ Fade Length";

    if (fade_length_in_ms > 0x7FFF) {
      fade_length_in_ms = fade_bytes[0] | (fade_bytes[1] << 8) |
                          (fade_bytes[2] << 16) | (fade_bytes[3] << 24);
    }
  }

  /*  Check for XID6 data -- this is infrequently used, but being able to fill
   * in data from this is ideal before trying to rely on APETAG values. XID6
   * format follows EA's binary file format standard named "IFF" */
  file.seek(XID6_OFFSET);
  if (has_id6 && file.read(4) == QString("xid6").toAscii()) {
    QByteArray xid6_head_data = file.read(4);
    if (xid6_head_data.size() >= 4) {
      qint64 xid6_size = xid6_head_data[0] | (xid6_head_data[1] << 8) |
                         (xid6_head_data[2] << 16) | xid6_head_data[3];
      /* This should be the size remaining for entire ID6 block, but it
       * seems that most files treat this as the size of the remaining header
       * space... */

      qLog(Debug) << file_info.fileName() << " has ID6 tag.";

      while ((file.pos()) + 4 < XID6_OFFSET + xid6_size) {
        QByteArray arr = file.read(4);
        if (arr.size() < 4) break;

        qint8 id = arr[0];
        qint8 type = arr[1];
        qint16 length = arr[2] | (arr[3] << 8);

        file.read(GetNextMemAddressAlign32bit(length));
      }
    }
  }

  /* Music Players that support SPC tend to support additional tagging data as
   * an APETAG entry at the bottom of the file instead of writing into the xid6
   * tagging space. This is where a lot of the extra data for a file is stored,
   * such as genre or replaygain data.
   *
   * This data is currently supported by TagLib, so we will simply use that for
   * the remaining values. */
  TagLib::APE::File ape(file_info.filePath().toStdString().data());
  if (ape.hasAPETag()) {
    TagLib::Tag* tag = ape.tag();
    if (!tag) return;

    song_info->set_year(tag->year());
    song_info->set_track(tag->track());
    TagReader::Decode(tag->artist(), nullptr, song_info->mutable_artist());
    TagReader::Decode(tag->title(), nullptr, song_info->mutable_title());
    TagReader::Decode(tag->album(), nullptr, song_info->mutable_album());
    TagReader::Decode(tag->genre(), nullptr, song_info->mutable_genre());
    song_info->set_valid(true);
  }

  song_info->set_type(pb::tagreader::SongMetadata_Type_SPC);
}

qint16 GME::SPC::GetNextMemAddressAlign32bit(qint16 input) {
  return ((input + 0x3) & ~0x3);
  // Plus 0x3 for rounding up (not down), AND NOT to flatten out on a 32 bit
  // level.
}

quint64 GME::SPC::ConvertSPCStringToNum(const QByteArray& arr) {
  quint64 result = 0;
  for (auto it = arr.begin(); it != arr.end(); it++) {
    unsigned int num = *it - '0';
    if (num > 9) break;
    result = (result * 10) + num;  // Shift Left and add.
  }
  return result;
}
