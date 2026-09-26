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

RendererCaps Caps(const QStringList& mime_types) {
  RendererCaps caps;
  caps.mime_types = mime_types;
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

TEST(StreamPlannerTest, SampleRateCapAddsACapsFilter) {
  RendererCaps caps = Caps({"audio/mpeg"});
  caps.max_sample_rate_hz = 48000;
  Song song = MakeSong(Song::Type_Flac, kLocalFlac);
  song.set_samplerate(96000);
  StreamPlan plan = Plan(song, caps);
  EXPECT_EQ(StreamPlan::Encode, plan.output);
  EXPECT_TRUE(plan.tail.contains("rate=(int)[1,48000]")) << plan.tail;
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
