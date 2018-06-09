#ifndef GMEREADER_H
#define GMEREADER_H

#include <QtCore>
#include "tagreadermessages.pb.h"

class QFileInfo;
class QByteArray;

namespace GME {
bool IsSupportedFormat(const QFileInfo& file_info);
void ReadFile(const QFileInfo& file_info,
              pb::tagreader::SongMetadata* song_info);

namespace SPC {
/* SPC SPEC:
 * http://vspcplay.raphnet.net/spc_file_format.txt
 */
const int HAS_ID6_OFFSET = 0x23;
const int SONG_TITLE_OFFSET = 0x2E;
const int GAME_TITLE_OFFSET = 0x4E;
const int DUMPER_OFFSET = 0x6E;
const int COMMENTS_OFFSET = 0x7E;
/*It seems that intro length and fade length are inconsistent from
 *file to file. It should be looked into within the GME source code
 *to see how GStreamer gets its values for playback length.*/
const int INTRO_LENGTH_OFFSET = 0xA9;
const int INTRO_LENGTH_SIZE = 3;
const int FADE_LENGTH_OFFSET = 0xAC;
const int FADE_LENGTH_SIZE = 4;
const int ARTIST_OFFSET = 0xB1;
const int XID6_OFFSET = (0x101C0 + 64);

const int NANO_PER_MS = 1000000;

enum xID6_STATUS {
  ON = 0x26,
  OFF = 0x27,
};

enum xID6_ID { SongName = 0x01, GameName = 0x02, ArtistName = 0x03 };

enum xID6_TYPE { Length = 0x0, String = 0x1, Integer = 0x4 };

void Read(const QFileInfo& file_info, pb::tagreader::SongMetadata* song_info);
qint16 GetNextMemAddressAlign32bit(qint16 input);
quint64 ConvertSPCStringToNum(const QByteArray& arr);
}  // namespace SPC
}  // namespace GME

#endif
