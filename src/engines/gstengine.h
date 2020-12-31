/***************************************************************************
 *   Copyright (C) 2003-2005 by Mark Kretschmann <markey@web.de>           *
 *   Copyright (C) 2005 by Jakub Stachowski <qbast@go2.pl>                 *
 *   Portions Copyright (C) 2006 Paul Cifarelli <paul@cifarelli.net>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#ifndef AMAROK_GSTENGINE_H
#define AMAROK_GSTENGINE_H

#include <gst/gst.h>

#include <QFuture>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTimerEvent>
#include <memory>

#include "bufferconsumer.h"
#include "core/timeconstants.h"
#include "enginebase.h"

class QTimer;
class QTimerEvent;

class Application;
class Console;
class DeviceFinder;
class GstEngineDebug;
class GstEnginePipeline;
class TaskManager;

#ifdef Q_OS_DARWIN
struct _GTlsDatabase;
typedef struct _GTlsDatabase GTlsDatabase;
#endif

/**
 * @class GstEngine
 * @short GStreamer engine plugin
 * @author Mark Kretschmann <markey@web.de>
 */
class GstEngine : public Engine::Base, public BufferConsumer {
  Q_OBJECT

 public:
  GstEngine(Application* app);
  ~GstEngine();

  struct OutputDetails {
    QString description;
    QString icon_name;

    QString gstreamer_plugin_name;
    QVariant device_property_value;
  };
  typedef QList<OutputDetails> OutputDetailsList;

  static const int kAutoSampleRate = -1;
  static const char* kSettingsGroup;
  static const char* kAutoSink;

  bool Init();
  void EnsureInitialised() { initialising_.waitForFinished(); }
  void InitialiseGstreamer();

  int AddBackgroundStream(const QUrl& url);
  void StopBackgroundStream(int id);
  void SetBackgroundStreamVolume(int id, int volume);

  qint64 position_nanosec() const;
  qint64 length_nanosec() const;
  Engine::State state() const;
  const Engine::Scope& scope(int chunk_length);

  OutputDetailsList GetOutputsList() const;

  GstElement* CreateElement(const QString& factoryName, GstElement* bin = 0);

  // BufferConsumer
  void ConsumeBuffer(GstBuffer* buffer, int pipeline_id);

 public slots:
  void StartPreloading(const MediaPlaybackRequest& req, bool force_stop_at_end,
                       qint64 beginning_nanosec, qint64 end_nanosec);
  bool Load(const MediaPlaybackRequest&, Engine::TrackChangeFlags change,
            bool force_stop_at_end, quint64 beginning_nanosec,
            qint64 end_nanosec);
  bool Play(quint64 offset_nanosec);
  void Stop(bool stop_after = false);
  void Pause();
  void Unpause();
  void Seek(quint64 offset_nanosec);

  /** Set whether equalizer is enabled */
  void SetEqualizerEnabled(bool);

  /** Set equalizer preamp and gains, range -100..100. Gains are 10 values. */
  void SetEqualizerParameters(int preamp, const QList<int>& bandGains);

  /** Set Stereo balance, range -1.0f..1.0f */
  void SetStereoBalance(float value);

  void ReloadSettings();

  void AddBufferConsumer(BufferConsumer* consumer);
  void RemoveBufferConsumer(BufferConsumer* consumer);

#ifdef Q_OS_DARWIN
  GTlsDatabase* tls_database() const { return tls_database_; }
#endif

  void NewDebugConsole(Console* console);

 protected:
  void SetVolumeSW(uint percent);
  void timerEvent(QTimerEvent*);

 private:
  // Used for debug purposes only.
  friend class GstEngineDebug;
  std::shared_ptr<GstEnginePipeline> GetCurrentPipeline() {
    return current_pipeline_;
  }

 private slots:
  void EndOfStreamReached(int pipeline_id, bool has_next_track);
  void HandlePipelineError(int pipeline_id, const QString& message, int domain,
                           int error_code);
  void NewMetaData(int pipeline_id, const Engine::SimpleMetaBundle& bundle);
  void AddBufferToScope(GstBuffer* buf, int pipeline_id);
  void FadeoutFinished();
  void FadeoutPauseFinished();
  void SeekNow();
  void BackgroundStreamFinished();
  void BackgroundStreamPlayDone(QFuture<GstStateChangeReturn>, int);
  void PlayDone(QFuture<GstStateChangeReturn> future, const quint64, const int);

  void BufferingStarted();
  void BufferingProgress(int percent);
  void BufferingFinished();

 private:
  struct PluginDetails {
    QString name;
    QString description;
  };

  typedef QList<PluginDetails> PluginDetailsList;

  PluginDetailsList GetPluginList(const QString& classname) const;

  void StartFadeout();
  void StartFadeoutPause();

  void StartTimers();
  void StopTimers();

  std::shared_ptr<GstEnginePipeline> CreatePipeline();
  std::shared_ptr<GstEnginePipeline> CreatePipeline(
      const MediaPlaybackRequest& req, qint64 end_nanosec);

  void UpdateScope(int chunk_length);

  int AddBackgroundStream(std::shared_ptr<GstEnginePipeline> pipeline);

  bool IsCurrentPipeline(int id);

 private:
  static const qint64 kTimerIntervalNanosec = 1000 * kNsecPerMsec;  // 1s
  static const qint64 kPreloadGapNanosec = 2000 * kNsecPerMsec;     // 2s
  static const qint64 kSeekDelayNanosec = 100 * kNsecPerMsec;       // 100msec

  static const char* kHypnotoadPipeline;
  static const char* kEnterprisePipeline;

  TaskManager* task_manager_;
  int buffering_task_id_;

  QFuture<void> initialising_;

  QString sink_;
  QVariant device_;

  std::shared_ptr<GstEnginePipeline> current_pipeline_;
  std::shared_ptr<GstEnginePipeline> fadeout_pipeline_;
  std::shared_ptr<GstEnginePipeline> fadeout_pause_pipeline_;
  QUrl preloaded_url_;

  QList<BufferConsumer*> buffer_consumers_;

  GstBuffer* latest_buffer_;

  bool equalizer_enabled_;
  int equalizer_preamp_;
  QList<int> equalizer_gains_;
  float stereo_balance_;

  bool rg_enabled_;
  int rg_mode_;
  float rg_preamp_;
  bool rg_compression_;

  qint64 buffer_duration_nanosec_;

  int buffer_min_fill_;

  bool mono_playback_;
  int sample_rate_;

  mutable bool can_decode_success_;
  mutable bool can_decode_last_;

  // Hack to stop seeks happening too often
  QTimer* seek_timer_;
  bool waiting_to_seek_;
  quint64 seek_pos_;

  int timer_id_;
  int next_element_id_;

  QHash<int, std::shared_ptr<GstEnginePipeline>> background_streams_;

  bool is_fading_out_to_pause_;
  bool has_faded_out_;

  int scope_chunk_;
  bool have_new_buffer_;
  int scope_chunks_;

  QList<DeviceFinder*> device_finders_;

#ifdef Q_OS_DARWIN
  GTlsDatabase* tls_database_;
#endif
};

Q_DECLARE_METATYPE(GstEngine::OutputDetails)

#endif /*AMAROK_GSTENGINE_H*/
