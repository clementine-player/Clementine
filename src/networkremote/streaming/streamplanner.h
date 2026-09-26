/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef NETWORKREMOTE_STREAMING_STREAMPLANNER_H_
#define NETWORKREMOTE_STREAMING_STREAMPLANNER_H_

#include <QList>
#include <QString>
#include <QStringList>

#include "core/song.h"
#include "engines/playbackrequest.h"

namespace cpb {
namespace remote {
class RendererCapabilities;
}
}  // namespace cpb

// What a renderer said it can play, in a form the planner can use without
// protobuf.
struct RendererCaps {
  struct Format {
    QString mime_type;
    QList<int> sample_rates_hz;  // empty = any
    int max_channels = 0;        // 0 = any

    bool AcceptsSampleRate(int hz) const {
      return sample_rates_hz.isEmpty() || hz <= 0 ||
             sample_rates_hz.contains(hz);
    }
  };

  QList<Format> formats;
  int max_bitrate_kbps = 0;  // 0 = no limit
  bool gapless = false;
  bool http_range = false;

  // Limits on what a renderer may describe. The planner renders a format's
  // sample rates into every pipeline it builds, so they're kept small.
  static const int kMaxFormats;
  static const int kMaxSampleRates;
  static const int kMaxMimeTypeLength;
  static const int kMaxSampleRateHz;
  static const int kMaxChannels;

  // Converts what a renderer sent, dropping anything out of range: formats
  // past kMaxFormats or with an unusable MIME type, sample rates past
  // kMaxSampleRates or not in 1..kMaxSampleRateHz, and negative limits.
  static RendererCaps FromProto(const cpb::remote::RendererCapabilities& pb);

  // The format that accepts |mime_type|, or nullptr. An entry without a
  // codecs= parameter accepts any codec in that container.
  const Format* Find(const QString& mime_type) const;
  bool Accepts(const QString& mime_type) const {
    return Find(mime_type) != nullptr;
  }

  QStringList mime_types() const;
};

struct StreamSettings {
  // Convert lossless files even when the renderer could play them. Shared
  // with the remote's download setting.
  bool transcode_lossless = false;
  // Decode and encode even if the renderer could play the source. Used to
  // retry an item the renderer said it could play, but couldn't.
  bool force_encode = false;
};

// How one track is delivered to a renderer. See docs/design/remote-streaming.md
// section 4.3.
struct StreamPlan {
  enum Mode {
    // Nothing the renderer supports can be produced.
    Unplayable,
    // The original local file, byte for byte.
    Direct,
    // Through a GStreamer pipeline.
    Pipeline,
  };
  enum Output {
    // Remux the source codec without decoding it.
    Passthrough,
    // Decode and encode.
    Encode,
  };

  Mode mode = Unplayable;
  Output output = Encode;

  // What the renderer receives.
  QString mime_type;

  // Pipeline only: caps at which uridecodebin stops decoding. "audio/x-raw"
  // for Encode.
  QString decode_caps;
  // Pipeline only: gst-launch description of the elements between the
  // decoder and the appsink.
  QString tail;

  // Where the track starts and ends within the source, for cue sheets.
  qint64 beginning_nanosec = 0;
  qint64 end_nanosec = 0;  // <= 0: to the end of the source
  // Length the renderer shows. <= 0 if unknown (radio).
  qint64 length_nanosec = 0;

  // Human readable reason, for logging.
  QString reason;
};

class StreamPlanner {
 public:
  static StreamPlan Plan(const MediaPlaybackRequest& req, const Song& song,
                         const RendererCaps& caps,
                         const StreamSettings& settings);
};

#endif  // NETWORKREMOTE_STREAMING_STREAMPLANNER_H_
