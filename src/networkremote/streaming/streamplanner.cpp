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

#include "streamplanner.h"

#include "remotecontrolmessages.pb.h"

namespace {

// What we know about a source codec.
struct CodecInfo {
  Song::FileType type;
  // MIME type of the original file, for Direct. Empty if never served as-is.
  const char* file_mime;
  // uridecodebin caps that stop decoding at this codec, for Passthrough.
  // Empty if there's no remux entry.
  const char* passthrough_caps;
  // Parser and muxer that make a streamable container of it.
  const char* passthrough_tail;
  // MIME type of the Passthrough output.
  const char* passthrough_mime;
};

const CodecInfo kCodecs[] = {
    {Song::Type_Mpeg, "audio/mpeg", "audio/mpeg, mpegversion=(int)1",
     "mpegaudioparse", "audio/mpeg"},
    {Song::Type_Flac, "audio/flac", "audio/x-flac", "flacparse", "audio/flac"},
    {Song::Type_OggVorbis, "audio/ogg; codecs=vorbis", "audio/x-vorbis",
     "vorbisparse ! oggmux", "audio/ogg; codecs=vorbis"},
    {Song::Type_OggOpus, "audio/ogg; codecs=opus", "audio/x-opus",
     "opusparse ! oggmux", "audio/ogg; codecs=opus"},
    {Song::Type_Mp4, "audio/mp4", "audio/mpeg, mpegversion=(int)4",
     "aacparse ! audio/mpeg, stream-format=(string)adts", "audio/aac"},
    {Song::Type_OggFlac, "audio/ogg; codecs=flac", "", "", ""},
    {Song::Type_Wav, "audio/wav", "", "", ""},
    {Song::Type_Aiff, "audio/aiff", "", "", ""},
};

const CodecInfo* FindCodec(Song::FileType type) {
  for (const CodecInfo& codec : kCodecs) {
    if (codec.type == type) return &codec;
  }
  return nullptr;
}

// Encoders in order of preference. %1 is the bitrate, in the unit the
// encoder's property takes.
struct EncodeTarget {
  const char* mime;
  const char* tail;
  int default_kbps;
  int bitrate_multiplier;
};

const EncodeTarget kEncodeTargets[] = {
    {"audio/ogg; codecs=opus", "opusenc bitrate=%1 ! oggmux", 128, 1000},
    {"audio/mpeg", "lamemp3enc target=bitrate cbr=true bitrate=%1", 192, 1},
    {"audio/ogg; codecs=vorbis", "vorbisenc bitrate=%1 ! oggmux", 160, 1000},
};

// "audio/ogg; codecs=opus" -> ("audio/ogg", "opus")
void SplitMime(const QString& mime, QString* type, QString* codecs) {
  const QStringList parts = mime.split(';');
  *type = parts[0].trimmed().toLower();
  codecs->clear();
  for (int i = 1; i < parts.size(); ++i) {
    const QString param = parts[i].trimmed();
    if (param.startsWith("codecs=", Qt::CaseInsensitive)) {
      *codecs = param.mid(7).remove('"').trimmed().toLower();
    }
  }
}

}  // namespace

RendererCaps RendererCaps::FromProto(
    const cpb::remote::RendererCapabilities& pb) {
  RendererCaps caps;
  for (const cpb::remote::AudioFormat& pb_format : pb.formats()) {
    Format format;
    format.mime_type = QString::fromStdString(pb_format.mime_type());
    for (int hz : pb_format.sample_rates_hz()) format.sample_rates_hz << hz;
    format.max_channels = pb_format.max_channels();
    caps.formats << format;
  }
  caps.max_bitrate_kbps = pb.max_bitrate_kbps();
  for (int feature : pb.features()) {
    switch (feature) {
      case cpb::remote::RENDERER_FEATURE_GAPLESS:
        caps.gapless = true;
        break;
      case cpb::remote::RENDERER_FEATURE_HTTP_RANGE:
        caps.http_range = true;
        break;
      default:
        break;
    }
  }
  return caps;
}

const RendererCaps::Format* RendererCaps::Find(const QString& mime_type) const {
  QString want_type, want_codecs;
  SplitMime(mime_type, &want_type, &want_codecs);

  for (const Format& format : formats) {
    QString type, codecs;
    SplitMime(format.mime_type, &type, &codecs);
    if (type != want_type) continue;
    if (codecs.isEmpty() || codecs == want_codecs) return &format;
  }
  return nullptr;
}

QStringList RendererCaps::mime_types() const {
  QStringList ret;
  for (const Format& format : formats) ret << format.mime_type;
  return ret;
}

StreamPlan StreamPlanner::Plan(const MediaPlaybackRequest& req,
                               const Song& song, const RendererCaps& caps,
                               const StreamSettings& settings) {
  StreamPlan plan;
  plan.beginning_nanosec = song.beginning_nanosec();
  plan.end_nanosec = song.end_nanosec();
  plan.length_nanosec = song.length_nanosec();

  const CodecInfo* codec = FindCodec(song.filetype());
  const bool cut = song.has_cue() || song.beginning_nanosec() > 0;
  const bool over_bitrate = caps.max_bitrate_kbps > 0 && song.bitrate() > 0 &&
                            song.bitrate() > caps.max_bitrate_kbps;
  const bool convert_lossless =
      settings.transcode_lossless && song.IsFileLossless();
  const bool as_is =
      !cut && !over_bitrate && !convert_lossless && !settings.force_encode;

  // Whether the renderer plays |mime_type| at the song's sample rate.
  bool rate_refused = false;
  auto plays = [&](const char* mime_type) {
    const RendererCaps::Format* format = caps.Find(mime_type);
    if (!format) return false;
    if (!format->AcceptsSampleRate(song.samplerate())) {
      rate_refused = true;
      return false;
    }
    return true;
  };

  // 1. Direct: a local file the renderer can play, with nothing to change.
  if (as_is && req.MediaUrl().isLocalFile() && codec && *codec->file_mime &&
      plays(codec->file_mime)) {
    plan.mode = StreamPlan::Direct;
    plan.mime_type = codec->file_mime;
    plan.reason = "local file the renderer can play";
    return plan;
  }

  plan.mode = StreamPlan::Pipeline;

  // 2. Passthrough: the renderer can play the source codec, and nothing
  // needs the decoded audio.
  if (as_is && codec && *codec->passthrough_caps &&
      plays(codec->passthrough_mime)) {
    plan.output = StreamPlan::Passthrough;
    plan.mime_type = codec->passthrough_mime;
    plan.decode_caps = codec->passthrough_caps;
    plan.tail = codec->passthrough_tail;
    plan.reason = "remux: the renderer can play the codec";
    return plan;
  }

  // 3. Encode to the first format the renderer accepts.
  for (const EncodeTarget& target : kEncodeTargets) {
    const RendererCaps::Format* format = caps.Find(target.mime);
    if (!format) continue;

    int kbps = target.default_kbps;
    if (caps.max_bitrate_kbps > 0) kbps = qMin(kbps, caps.max_bitrate_kbps);

    // audioresample picks the allowed rate nearest the source's.
    QString raw = "audioconvert ! audioresample";
    QStringList limits;
    if (!format->sample_rates_hz.isEmpty()) {
      QStringList rates;
      for (int hz : format->sample_rates_hz) rates << QString::number(hz);
      limits << QString("rate=(int){ %1 }").arg(rates.join(", "));
    }
    if (format->max_channels > 0) {
      limits << QString("channels=(int)[1,%1]").arg(format->max_channels);
    }
    if (!limits.isEmpty()) {
      raw += " ! audio/x-raw, " + limits.join(", ");
    }

    plan.output = StreamPlan::Encode;
    plan.mime_type = target.mime;
    plan.decode_caps = "audio/x-raw";
    plan.tail = raw + " ! " +
                QString(target.tail).arg(kbps * target.bitrate_multiplier);
    plan.reason = settings.force_encode ? "encode: retrying after an error"
                  : cut              ? "encode: track is cut from a larger file"
                  : convert_lossless ? "encode: converting a lossless file"
                  : over_bitrate     ? "encode: over the renderer's bitrate"
                  : rate_refused
                      ? "encode: the renderer can't play the sample rate"
                  : !codec ? "encode: unknown source codec"
                           : "encode: the renderer can't play the codec";
    return plan;
  }

  plan.mode = StreamPlan::Unplayable;
  plan.reason = "the renderer accepts none of the formats we can encode";
  return plan;
}
