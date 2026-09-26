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

#include "core/song.h"
#include "gtest/gtest.h"
#include "networkremote/protocolsniffer.h"
#include "networkremote/streaming/mediahttpserver.h"
#include "networkremote/streaming/pipelineresponder.h"
#include "networkremote/streaming/streamplanner.h"
#include "remotecontrolmessages.pb.h"
#include "test_utils.h"

namespace {

Song MakeSong(Song::FileType type, const QUrl& url) {
  Song song;
  song.set_filetype(type);
  song.set_url(url);
  song.set_length_nanosec(180ll * 1000 * 1000 * 1000);
  song.set_bitrate(256);
  song.set_samplerate(44100);
  return song;
}

RendererCaps::Format Format(const QString& mime_type,
                            const QList<int>& sample_rates_hz = {},
                            int max_channels = 0) {
  RendererCaps::Format format;
  format.mime_type = mime_type;
  format.sample_rates_hz = sample_rates_hz;
  format.max_channels = max_channels;
  return format;
}

RendererCaps Caps(const QStringList& mime_types) {
  RendererCaps caps;
  for (const QString& mime_type : mime_types) caps.formats << Format(mime_type);
  return caps;
}

StreamPlan Plan(const Song& song, const RendererCaps& caps,
                const StreamSettings& settings = StreamSettings()) {
  return StreamPlanner::Plan(MediaPlaybackRequest(song.url()), song, caps,
                             settings);
}

const QUrl kLocalMp3 = QUrl::fromLocalFile("/music/a.mp3");
const QUrl kLocalFlac = QUrl::fromLocalFile("/music/a.flac");
const QUrl kRemoteMp3 = QUrl("https://example.com/a.mp3");

}  // namespace

TEST(RendererCapsTest, ContainerWithoutCodecsAcceptsAnyCodec) {
  RendererCaps caps = Caps({"audio/ogg"});
  EXPECT_TRUE(caps.Accepts("audio/ogg; codecs=opus"));
  EXPECT_TRUE(caps.Accepts("audio/ogg; codecs=vorbis"));
  EXPECT_FALSE(caps.Accepts("audio/mpeg"));
}

TEST(RendererCapsTest, FromProtoDropsWhatsOutOfRange) {
  cpb::remote::RendererCapabilities pb;
  for (int i = 0; i < RendererCaps::kMaxFormats + 10; ++i) {
    pb.add_formats()->set_mime_type("audio/mpeg");
  }
  cpb::remote::AudioFormat* first = pb.mutable_formats(0);
  first->add_sample_rates_hz(-44100);
  first->add_sample_rates_hz(0);
  first->add_sample_rates_hz(48000);
  first->add_sample_rates_hz(RendererCaps::kMaxSampleRateHz + 1);
  for (int i = 0; i < RendererCaps::kMaxSampleRates + 10; ++i) {
    first->add_sample_rates_hz(8000 + i);
  }
  first->set_max_channels(-2);
  pb.mutable_formats(1)->set_max_channels(1000);
  pb.set_max_bitrate_kbps(-5);

  RendererCaps caps = RendererCaps::FromProto(pb);
  ASSERT_EQ(RendererCaps::kMaxFormats, caps.formats.size());
  const QList<int>& rates = caps.formats[0].sample_rates_hz;
  EXPECT_EQ(RendererCaps::kMaxSampleRates, rates.size());
  EXPECT_EQ(48000, rates[0]);
  for (int hz : rates) {
    EXPECT_GT(hz, 0);
    EXPECT_LE(hz, RendererCaps::kMaxSampleRateHz);
  }
  EXPECT_EQ(0, caps.formats[0].max_channels);
  EXPECT_EQ(RendererCaps::kMaxChannels, caps.formats[1].max_channels);
  EXPECT_EQ(0, caps.max_bitrate_kbps);
}

TEST(RendererCapsTest, FromProtoDropsFormatsWithOnlyInvalidRates) {
  // An empty list would mean "any rate", which is the opposite of what the
  // renderer sent.
  cpb::remote::RendererCapabilities pb;
  cpb::remote::AudioFormat* bad = pb.add_formats();
  bad->set_mime_type("audio/flac");
  bad->add_sample_rates_hz(-1);
  pb.add_formats()->set_mime_type(std::string(1000, 'x'));
  pb.add_formats()->set_mime_type("audio/mpeg");

  RendererCaps caps = RendererCaps::FromProto(pb);
  ASSERT_EQ(1, caps.formats.size());
  EXPECT_EQ("audio/mpeg", caps.formats[0].mime_type);
}

TEST(RendererCapsTest, CodecsMustMatch) {
  RendererCaps caps = Caps({"audio/ogg; codecs=\"vorbis\""});
  EXPECT_TRUE(caps.Accepts("audio/ogg; codecs=vorbis"));
  EXPECT_FALSE(caps.Accepts("audio/ogg; codecs=opus"));
}

TEST(StreamPlannerTest, LocalFileTheRendererPlaysIsDirect) {
  StreamPlan plan =
      Plan(MakeSong(Song::Type_Mpeg, kLocalMp3), Caps({"audio/mpeg"}));
  EXPECT_EQ(StreamPlan::Direct, plan.mode);
  EXPECT_EQ("audio/mpeg", plan.mime_type);
}

TEST(StreamPlannerTest, UnsupportedCodecIsEncodedToThePreferredFormat) {
  StreamPlan plan = Plan(MakeSong(Song::Type_Flac, kLocalFlac),
                         Caps({"audio/mpeg", "audio/ogg; codecs=opus"}));
  EXPECT_EQ(StreamPlan::Pipeline, plan.mode);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
  EXPECT_EQ("audio/ogg; codecs=opus", plan.mime_type);
  EXPECT_TRUE(plan.tail.contains("opusenc bitrate=128000"));
}

TEST(StreamPlannerTest, LosslessIsConvertedWhenAsked) {
  StreamSettings settings;
  settings.transcode_lossless = true;
  StreamPlan plan = Plan(MakeSong(Song::Type_Flac, kLocalFlac),
                         Caps({"audio/flac", "audio/mpeg"}), settings);
  EXPECT_EQ(StreamPlan::Pipeline, plan.mode);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
  EXPECT_EQ("audio/mpeg", plan.mime_type);
}

TEST(StreamPlannerTest, CueTrackIsEncoded) {
  Song song = MakeSong(Song::Type_Mpeg, kLocalMp3);
  song.set_beginning_nanosec(60ll * 1000 * 1000 * 1000);
  StreamPlan plan = Plan(song, Caps({"audio/mpeg"}));
  EXPECT_EQ(StreamPlan::Pipeline, plan.mode);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
  EXPECT_EQ(song.beginning_nanosec(), plan.beginning_nanosec);
}

TEST(StreamPlannerTest, RemoteFileTheRendererPlaysIsRemuxed) {
  StreamPlan plan =
      Plan(MakeSong(Song::Type_Mpeg, kRemoteMp3), Caps({"audio/mpeg"}));
  EXPECT_EQ(StreamPlan::Pipeline, plan.mode);
  EXPECT_EQ(StreamPlan::Passthrough, plan.output);
  EXPECT_EQ("audio/mpeg", plan.mime_type);
  EXPECT_TRUE(plan.decode_caps.startsWith("audio/mpeg"));
  EXPECT_EQ("mpegaudioparse", plan.tail);
}

TEST(StreamPlannerTest, UnknownStreamIsEncoded) {
  StreamPlan plan =
      Plan(MakeSong(Song::Type_Stream, QUrl("http://radio.example/live")),
           Caps({"audio/mpeg"}));
  EXPECT_EQ(StreamPlan::Pipeline, plan.mode);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
}

TEST(StreamPlannerTest, BitrateCapLimitsTheEncoder) {
  RendererCaps caps = Caps({"audio/ogg; codecs=opus"});
  caps.max_bitrate_kbps = 96;
  StreamPlan plan = Plan(MakeSong(Song::Type_OggOpus, kLocalMp3), caps);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
  EXPECT_TRUE(plan.tail.contains("bitrate=96000")) << plan.tail;
}

TEST(StreamPlannerTest, UnlistedSampleRateIsResampled) {
  // Plays FLAC, but not at 96 kHz.
  RendererCaps caps;
  caps.formats << Format("audio/flac", {44100, 48000})
               << Format("audio/mpeg", {44100, 48000}, 2);
  Song song = MakeSong(Song::Type_Flac, kLocalFlac);
  song.set_samplerate(96000);

  StreamPlan plan = Plan(song, caps);
  EXPECT_EQ(StreamPlan::Pipeline, plan.mode);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
  EXPECT_EQ("audio/mpeg", plan.mime_type);
  EXPECT_TRUE(plan.tail.contains("rate=(int){ 44100, 48000 }")) << plan.tail;
  EXPECT_TRUE(plan.tail.contains("channels=(int)[1,2]")) << plan.tail;
}

TEST(StreamPlannerTest, ListedSampleRateIsDirect) {
  RendererCaps caps;
  caps.formats << Format("audio/flac", {44100, 48000});
  StreamPlan plan = Plan(MakeSong(Song::Type_Flac, kLocalFlac), caps);
  EXPECT_EQ(StreamPlan::Direct, plan.mode);
}

TEST(StreamPlannerTest, UnknownSampleRateIsAccepted) {
  RendererCaps caps;
  caps.formats << Format("audio/flac", {48000});
  Song song = MakeSong(Song::Type_Flac, kLocalFlac);
  song.set_samplerate(0);
  EXPECT_EQ(StreamPlan::Direct, Plan(song, caps).mode);
}

TEST(StreamPlannerTest, LimitsAreChosenPerFormat) {
  // Opus is preferred, but it's the MP3 entry's limits that don't apply.
  RendererCaps caps;
  caps.formats << Format("audio/mpeg", {44100}, 1)
               << Format("audio/ogg; codecs=opus", {48000});
  StreamPlan plan = Plan(MakeSong(Song::Type_Flac, kLocalFlac), caps);
  EXPECT_EQ("audio/ogg; codecs=opus", plan.mime_type);
  EXPECT_TRUE(plan.tail.contains("rate=(int){ 48000 }")) << plan.tail;
  EXPECT_FALSE(plan.tail.contains("channels")) << plan.tail;
}

TEST(StreamPlannerTest, ForceEncodeSkipsDirect) {
  StreamSettings settings;
  settings.force_encode = true;
  StreamPlan plan = Plan(MakeSong(Song::Type_Mpeg, kLocalMp3),
                         Caps({"audio/mpeg"}), settings);
  EXPECT_EQ(StreamPlan::Pipeline, plan.mode);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
}

TEST(StreamPlannerTest, NothingWeCanProduceIsUnplayable) {
  StreamPlan plan =
      Plan(MakeSong(Song::Type_Mpeg, kLocalMp3), Caps({"audio/flac"}));
  EXPECT_EQ(StreamPlan::Unplayable, plan.mode);
}

TEST(PipelineResponderTest, DescriptionEndsInAnAppsink) {
  StreamPlan plan =
      Plan(MakeSong(Song::Type_Mpeg, kRemoteMp3), Caps({"audio/mpeg"}));
  const QString description = PipelineResponder::Description(plan);
  EXPECT_TRUE(description.startsWith(
      "uridecodebin name=decode caps=\"audio/mpeg, mpegversion=(int)1\" ! "
      "mpegaudioparse ! appsink name=sink"))
      << description;
}

TEST(ProtocolSnifferTest, RemoteLengthPrefix) {
  EXPECT_EQ(ProtocolSniffer::Remote, ProtocolSniffer::Classify(0x00));
  EXPECT_EQ(ProtocolSniffer::Remote, ProtocolSniffer::Classify(0x08));
}

TEST(ProtocolSnifferTest, HttpMethods) {
  EXPECT_EQ(ProtocolSniffer::Http, ProtocolSniffer::Classify('G'));
  EXPECT_EQ(ProtocolSniffer::Http, ProtocolSniffer::Classify('H'));
  // The h2c preface ("PRI * HTTP/2.0") is left to the HTTP handler.
  EXPECT_EQ(ProtocolSniffer::Http, ProtocolSniffer::Classify('P'));
}

TEST(ProtocolSnifferTest, EverythingElseIsUnknown) {
  EXPECT_EQ(ProtocolSniffer::Unknown, ProtocolSniffer::Classify(0x09));
  EXPECT_EQ(ProtocolSniffer::Unknown, ProtocolSniffer::Classify(0x16));  // TLS
  EXPECT_EQ(ProtocolSniffer::Unknown, ProtocolSniffer::Classify('g'));
  EXPECT_EQ(ProtocolSniffer::Unknown, ProtocolSniffer::Classify(0xff));
}

TEST(HttpRequestTest, ParsesRequestLineAndHeaders) {
  HttpRequest request;
  ASSERT_TRUE(HttpRequest::Parse(
      "GET /s/abc/3?t=1500 HTTP/1.1\r\nHost: x\r\nRange: bytes=0-\r\n",
      &request));
  EXPECT_EQ("GET", QString(request.method));
  EXPECT_EQ("/s/abc/3", QString(request.path));
  EXPECT_EQ("t=1500", QString(request.query));
  EXPECT_EQ("bytes=0-", QString(request.headers.value("range")));
  EXPECT_EQ("x", QString(request.headers.value("host")));
}

TEST(HttpRequestTest, RejectsGarbage) {
  HttpRequest request;
  EXPECT_FALSE(HttpRequest::Parse("GET /\r\n", &request));
  EXPECT_FALSE(HttpRequest::Parse("GET / HTTP/1.1\r\nNoColon\r\n", &request));
  EXPECT_FALSE(HttpRequest::Parse("PRI * HTTP/2.0\r\n", &request));
}

TEST(ByteRangeTest, NoHeaderMeansWholeFile) {
  qint64 first, last;
  EXPECT_EQ(ByteRange::Whole, ByteRange::Parse("", 100, &first, &last));
  EXPECT_EQ(ByteRange::Whole,
            ByteRange::Parse("bytes=0-1,5-6", 100, &first, &last));
}

TEST(ByteRangeTest, OpenAndClosedRanges) {
  qint64 first, last;
  ASSERT_EQ(ByteRange::Partial,
            ByteRange::Parse("bytes=0-", 100, &first, &last));
  EXPECT_EQ(0, first);
  EXPECT_EQ(99, last);

  ASSERT_EQ(ByteRange::Partial,
            ByteRange::Parse("bytes=10-19", 100, &first, &last));
  EXPECT_EQ(10, first);
  EXPECT_EQ(19, last);

  // Past the end is clamped.
  ASSERT_EQ(ByteRange::Partial,
            ByteRange::Parse("bytes=90-500", 100, &first, &last));
  EXPECT_EQ(99, last);
}

TEST(ByteRangeTest, SuffixRange) {
  qint64 first, last;
  ASSERT_EQ(ByteRange::Partial,
            ByteRange::Parse("bytes=-10", 100, &first, &last));
  EXPECT_EQ(90, first);
  EXPECT_EQ(99, last);
}

TEST(ByteRangeTest, Unsatisfiable) {
  qint64 first, last;
  EXPECT_EQ(ByteRange::Unsatisfiable,
            ByteRange::Parse("bytes=100-", 100, &first, &last));
  EXPECT_EQ(ByteRange::Unsatisfiable,
            ByteRange::Parse("bytes=20-10", 100, &first, &last));
  EXPECT_EQ(ByteRange::Unsatisfiable,
            ByteRange::Parse("bytes=-0", 100, &first, &last));
}
