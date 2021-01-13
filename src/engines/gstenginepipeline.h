/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef GSTENGINEPIPELINE_H
#define GSTENGINEPIPELINE_H

#include <QBasicTimer>
#include <QFuture>
#include <QMutex>
#include <QThreadPool>
#include <QTimeLine>
#include <QUrl>
#include <memory>

#include "engine_fwd.h"
#include "gstpipelinebase.h"
#include "playbackrequest.h"

class GstElementDeleter;
class GstEngine;
class BufferConsumer;

struct GstQueue;
struct GstURIDecodeBin;

class GstEnginePipeline : public GstPipelineBase {
  Q_OBJECT

 public:
  GstEnginePipeline(GstEngine* engine);
  ~GstEnginePipeline();

  // Call these setters before Init
  void set_output_device(const QString& sink, const QVariant& device);
  void set_replaygain(bool enabled, int mode, float preamp, bool compression);
  void set_buffer_duration_nanosec(qint64 duration_nanosec);
  void set_buffer_min_fill(int percent);
  void set_mono_playback(bool enabled);
  void set_sample_rate(int rate);

  // Creates the pipeline, returns false on error
  bool InitFromReq(const MediaPlaybackRequest& req, qint64 end_nanosec);
  bool InitFromString(const QString& pipeline);

  // BufferConsumers get fed audio data.  Thread-safe.
  void AddBufferConsumer(BufferConsumer* consumer);
  void RemoveBufferConsumer(BufferConsumer* consumer);
  void RemoveAllBufferConsumers();

  // Control the music playback
  QFuture<GstStateChangeReturn> SetState(GstState state);
  Q_INVOKABLE bool Seek(qint64 nanosec);
  void SetEqualizerEnabled(bool enabled);
  void SetEqualizerParams(int preamp, const QList<int>& band_gains);
  void SetVolume(int percent);
  void SetStereoBalance(float value);
  void StartFader(qint64 duration_nanosec,
                  QTimeLine::Direction direction = QTimeLine::Forward,
                  QTimeLine::CurveShape shape = QTimeLine::LinearCurve,
                  bool use_fudge_timer = true);

  // If this is set then it will be loaded automatically when playback finishes
  // for gapless playback
  void SetNextReq(const MediaPlaybackRequest& req, qint64 beginning_nanosec,
                  qint64 end_nanosec);
  bool has_next_valid_url() const { return next_.url_.isValid(); }

  // Get information about the music playback
  QUrl url() const { return current_.url_; }
  bool is_valid() const { return valid_; }
  // Please note that this method (unlike GstEngine's.position()) is
  // multiple-section media unaware.
  qint64 position() const;
  // Please note that this method (unlike GstEngine's.length()) is
  // multiple-section media unaware.
  qint64 length() const;
  // Returns this pipeline's state. May return GST_STATE_NULL if the state check
  // timed out. The timeout value is a reasonable default.
  GstState state() const;
  qint64 segment_start() const { return segment_start_; }

  // Don't allow the user to change the playback state (playing/paused) while
  // the pipeline is buffering.
  bool is_buffering() const { return buffering_; }

  QUrl redirect_url() const { return redirect_url_; }

  QString source_device() const { return source_device_; }

 public slots:
  void SetVolumeModifier(qreal mod);

 signals:
  void EndOfStreamReached(int pipeline_id, bool has_next_track);
  void MetadataFound(int pipeline_id, const Engine::SimpleMetaBundle& bundle);
  // This indicates an error, delegated from GStreamer, in the pipeline.
  // The message, domain and error_code are related to GStreamer's GError.
  void Error(int pipeline_id, const QString& message, int domain,
             int error_code);
  void FaderFinished();

  void BufferingStarted();
  void BufferingProgress(int percent);
  void BufferingFinished();

 protected:
  void timerEvent(QTimerEvent*);

 private:
  // Static callbacks.  The GstEnginePipeline instance is passed in the last
  // argument.
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage*, gpointer);
  static gboolean BusCallback(GstBus*, GstMessage*, gpointer);
  static void NewPadCallback(GstElement*, GstPad*, gpointer);
  static GstPadProbeReturn HandoffCallback(GstPad*, GstPadProbeInfo*, gpointer);
  static GstPadProbeReturn EventHandoffCallback(GstPad*, GstPadProbeInfo*,
                                                gpointer);
  static GstPadProbeReturn DecodebinProbe(GstPad*, GstPadProbeInfo*, gpointer);
  static void SourceDrainedCallback(GstURIDecodeBin*, gpointer);
  static void SourceSetupCallback(GstURIDecodeBin*, GParamSpec* pspec,
                                  gpointer);
  static void TaskEnterCallback(GstTask*, GThread*, gpointer);

  static QByteArray GstUriFromUrl(const QUrl& url);

  void TagMessageReceived(GstMessage*);
  void ErrorMessageReceived(GstMessage*);
  void ElementMessageReceived(GstMessage*);
  void StateChangedMessageReceived(GstMessage*);
  void BufferingMessageReceived(GstMessage*);
  void StreamStatusMessageReceived(GstMessage*);

  QString ParseTag(GstTagList* list, const char* tag) const;

  bool InitAudioBin();
  GstElement* CreateDecodeBinFromString(const char* pipeline);
  GstElement* CreateDecodeBinFromUrl(const QUrl& url);

  void UpdateVolume();
  void UpdateEqualizer();
  void UpdateStereoBalance();
  bool ReplaceDecodeBin(GstElement* new_bin);
  bool ReplaceDecodeBin(const QUrl& url);

  void TransitionToNext();

  // If the decodebin is special (ie. not really a uridecodebin) then it'll have
  // a src pad immediately and we can link it after everything's created.
  void MaybeLinkDecodeToAudio();

  // Helper method to retrieve the audio format from a GstCaps object.
  static QString GetAudioFormat(GstCaps* caps);

 private slots:
  void FaderTimelineFinished();

 private:
  static const int kGstStateTimeoutNanosecs;
  static const int kFaderFudgeMsec;
  static const int kEqBandCount;
  static const int kEqBandFrequencies[];

  static GstElementDeleter* sElementDeleter;

  GstEngine* engine_;

  // General settings for the pipeline
  bool valid_;
  QString sink_;
  QVariant device_;

  // These get called when there is a new audio buffer available
  QList<BufferConsumer*> buffer_consumers_;
  QMutex buffer_consumers_mutex_;
  qint64 segment_start_;
  bool segment_start_received_;
  bool emit_track_ended_on_stream_start_;
  bool emit_track_ended_on_time_discontinuity_;
  qint64 last_buffer_offset_;

  // Equalizer
  bool eq_enabled_;
  int eq_preamp_;
  QList<int> eq_band_gains_;

  // Stereo balance.
  // From -1.0 - 1.0
  // -1.0 is left, 1.0 is right.
  float stereo_balance_;

  // ReplayGain
  bool rg_enabled_;
  int rg_mode_;
  float rg_preamp_;
  bool rg_compression_;

  // Buffering
  quint64 buffer_duration_nanosec_;
  int buffer_min_fill_;
  bool buffering_;

  bool mono_playback_;
  int sample_rate_;

  // The URL that is currently playing, and the URL that is to be preloaded
  // when the current track is close to finishing.
  MediaPlaybackRequest current_;
  MediaPlaybackRequest next_;

  // If this is > 0 then the pipeline will be forced to stop when playback goes
  // past this position.
  qint64 end_offset_nanosec_;

  // We store the beginning and end for the preloading song too, so we can just
  // carry on without reloading the file if the sections carry on from each
  // other.
  qint64 next_beginning_offset_nanosec_;
  qint64 next_end_offset_nanosec_;

  // Set temporarily when moving to the next contiguous section in a multi-part
  // file.
  bool ignore_next_seek_;

  // Set temporarily when switching out the decode bin, so metadata doesn't
  // get sent while the Player still thinks it's playing the last song
  bool ignore_tags_;

  // When the gstreamer source requests a redirect we store the URL here and
  // callers can pick it up after the state change to PLAYING fails.
  QUrl redirect_url_;

  // When we need to specify the device to use as source (for CD device)
  QString source_device_;

  // Seeking while the pipeline is in the READY state doesn't work, so we have
  // to wait until it goes to PAUSED or PLAYING.
  // Also we have to wait for the decodebin to be connected.
  bool pipeline_is_initialised_;
  bool pipeline_is_connected_;
  qint64 pending_seek_nanosec_;

  // We can only use gst_element_query_position() when the pipeline is in
  // PAUSED nor PLAYING state. Whenever we get a new position (e.g. after a
  // correct call to gst_element_query_position() or after a seek), we store
  // it here so that we can use it when using gst_element_query_position() is
  // not possible.
  mutable gint64 last_known_position_ns_;

  int volume_percent_;
  qreal volume_modifier_;

  std::unique_ptr<QTimeLine> fader_;
  QBasicTimer fader_fudge_timer_;
  bool use_fudge_timer_;

  // Bins
  // uridecodebin ! audiobin
  GstElement* uridecodebin_;
  GstElement* audiobin_;

  // Elements in the audiobin.  See comments in Init()'s definition.
  GstElement* queue_;
  GstElement* audioconvert_;
  GstElement* rgvolume_;
  GstElement* rglimiter_;
  GstElement* audioconvert2_;
  GstElement* equalizer_preamp_;
  GstElement* equalizer_;
  GstElement* stereo_panorama_;
  GstElement* volume_;
  GstElement* audioscale_;
  GstElement* audiosink_;
  GstElement* capsfilter_;

  // tee and request pads.
  GstElement* tee_;
  GstPad* tee_probe_pad_;
  GstPad* tee_audio_pad_;

  uint bus_cb_id_;

  QThreadPool set_state_threadpool_;

  GstSegment last_decodebin_segment_;
};

#endif  // GSTENGINEPIPELINE_H
