#include "core/logging.h"
#include "core/timeconstants.h"
#include "gmereader.h"
#include "tagreader.h"

#include <apefile.h>
#include <tag.h>
#include <QByteArray>
#include <QChar>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtEndian>

bool GME::IsSupportedFormat(const QFileInfo& file_info) {
  return (file_info.completeSuffix().endsWith("spc") ||
          file_info.completeSuffix().endsWith("vgm"));
}

void GME::ReadFile(const QFileInfo& file_info,
                   pb::tagreader::SongMetadata* song_info) {
  if (file_info.completeSuffix().endsWith("spc"))
    SPC::Read(file_info, song_info);
  if (file_info.completeSuffix().endsWith("vgm"))
    VGM::Read(file_info, song_info);
}

void GME::SPC::Read(const QFileInfo& file_info,
                    pb::tagreader::SongMetadata* song_info) {
  QFile file(file_info.filePath());
  if (!file.open(QIODevice::ReadOnly)) return;

  qLog(Debug) << "Reading tags from SPC file: " << file_info.fileName();

  // Check for header -- more reliable than file name alone.
  if (!file.read(33).startsWith(QString("SNES-SPC700").toLatin1())) return;

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
  song_info->set_title(QString::fromLatin1(file.read(32)).toStdString());

  file.seek(GAME_TITLE_OFFSET);
  song_info->set_album(QString::fromLatin1(file.read(32)).toStdString());

  file.seek(ARTIST_OFFSET);
  song_info->set_artist(QString::fromLatin1(file.read(32)).toStdString());

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
  if (has_id6 && file.read(4) == QString("xid6").toLatin1()) {
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
  }

  song_info->set_valid(true);
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

void GME::VGM::Read(const QFileInfo& file_info,
                    pb::tagreader::SongMetadata* song_info) {
  QFile file(file_info.filePath());
  if (!file.open(QIODevice::ReadOnly)) return;

  qLog(Debug) << "Reading tags from VGM file: " << file_info.fileName();

  if (!file.read(4).startsWith(QString("Vgm ").toLatin1())) return;

  file.seek(GD3_TAG_PTR);
  QByteArray gd3_head = file.read(4);
  if (gd3_head.size() < 4) return;

  quint64 pt = (unsigned char)gd3_head[0] | ((unsigned char)gd3_head[1] << 8) |
               ((unsigned char)gd3_head[2] << 16) |
               ((unsigned)gd3_head[3] << 24);

  file.seek(SAMPLE_COUNT);
  QByteArray sample_count_bytes = file.read(4);
  file.seek(LOOP_SAMPLE_COUNT);
  QByteArray loop_count_bytes = file.read(4);
  quint64 length = 0;

  if (!GetPlaybackLength(sample_count_bytes, loop_count_bytes, length)) return;

  file.seek(GD3_TAG_PTR + pt);
  QByteArray gd3_version = file.read(4);

  file.seek(file.pos() + 4);
  QByteArray gd3_length_bytes = file.read(4);
  quint32 gd3_length =
      (unsigned char)gd3_length_bytes[0] | ((unsigned char)gd3_head[1] << 8) |
      ((unsigned char)gd3_head[2] << 16) | ((unsigned char)gd3_head[3] << 24);

  QByteArray gd3Data = file.read(gd3_length);
  QTextStream fileTagStream(gd3Data, QIODevice::ReadOnly);
  // Stored as 16 bit UTF string, two bytes per letter.
  fileTagStream.setCodec("UTF-16");
  QStringList strings = fileTagStream.readLine(0).split(QChar('\0'));
  if (strings.count() < 10) return;

  /* VGM standard dictates string tag data exist in specific order.
   * Order alternates between English and Japanese version of data.
   * Read GD3 tag standard for more details. */
  song_info->set_title(strings[0].toStdString());
  song_info->set_album(strings[2].toStdString());
  song_info->set_artist(strings[6].toStdString());
  song_info->set_year(strings[8].left(4).toInt());
  song_info->set_length_nanosec(length * kNsecPerMsec);
  song_info->set_valid(true);
  song_info->set_type(pb::tagreader::SongMetadata_Type_VGM);
}

bool GME::VGM::GetPlaybackLength(const QByteArray& sample_count_bytes,
                                 const QByteArray& loop_count_bytes,
                                 quint64& out_length) {
  if (sample_count_bytes.size() != 4) return false;
  if (loop_count_bytes.size() != 4) return false;

  quint64 sample_count = (unsigned char)sample_count_bytes[0] |
                         ((unsigned char)sample_count_bytes[1] << 8) |
                         ((unsigned char)sample_count_bytes[2] << 16) |
                         ((unsigned char)sample_count_bytes[3] << 24);

  qLog(Debug) << QString::number(sample_count, 16);
  qLog(Debug) << sample_count_bytes.toHex();

  if (sample_count <= 0) return false;

  quint64 loop_sample_count = (unsigned char)loop_count_bytes[0] |
                              ((unsigned char)loop_count_bytes[1] << 8) |
                              ((unsigned char)loop_count_bytes[2] << 16) |
                              ((unsigned char)loop_count_bytes[3] << 24);

  if (loop_sample_count <= 0) {
    out_length = sample_count * 1000 / SAMPLE_TIMEBASE;
    return true;
  }

  quint64 intro_length_ms =
      (sample_count - loop_sample_count) * 1000 / SAMPLE_TIMEBASE;
  quint64 loop_length_ms = (loop_sample_count)*1000 / SAMPLE_TIMEBASE;
  out_length = intro_length_ms + (loop_length_ms * 2) + GST_GME_LOOP_TIME_MS;
  return true;
}
