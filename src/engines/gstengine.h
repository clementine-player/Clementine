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

#include "enginebase.h"

#include <QString>
#include <QTimerEvent>
#include <QList>
#include <QStringList>
#include <QMutex>

#include <gst/gst.h>

class QTimerEvent;

/**
 * @class GstEngine
 * @short GStreamer engine plugin
 * @author Mark Kretschmann <markey@web.de>
 *
 * GstEngine uses following pipeline for playing (syntax used by gst-launch):
 * { filesrc location=file.ext ! decodebin ! audioconvert ! audioscale ! volume
 * ! adder } ! { queue ! equalizer ! identity ! volume ! audiosink }
 */
class GstEngine : public Engine::Base {
  Q_OBJECT

 public:
  GstEngine();
  ~GstEngine();

  struct PluginDetails {
    QString name;
    QString long_name;
    QString author;
    QString description;
  };
  typedef QList<PluginDetails> PluginDetailsList;

  static const char* kSettingsGroup;
  static const char* kAutoSink;

  bool init();

  bool canDecode(const QUrl& url);
  uint position() const;
  uint length() const;
  Engine::State state() const;
  const Engine::Scope& scope();

  virtual bool metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle &b);
  virtual bool getAudioCDContents(const QString &device, QList<QUrl> &urls);

  void gstStatusText(const QString& str) { emit statusText( str ); }
  void gstMetaData(Engine::SimpleMetaBundle &bundle) { emit metaData( bundle ); }

  PluginDetailsList GetOutputsList() const { return GetPluginList( "Sink/Audio" ); }

 public slots:
  bool load(const QUrl&, bool stream);
  bool play(uint offset);
  void stop();
  void pause();
  void unpause();
  void seek(uint ms);

  /** Set whether equalizer is enabled */
  void setEqualizerEnabled(bool);

  /** Set equalizer preamp and gains, range -100..100. Gains are 10 values. */
  void setEqualizerParameters(int preamp, const QList<int>& bandGains);

  void ReloadSettings();

 protected:
  void setVolumeSW(uint percent);
  void timerEvent(QTimerEvent*);

 private slots:
  void HandlePipelineError();
  void EndOfStreamReached();

  /** Called when no output sink was selected. Shows the GStreamer engine settings dialog. */
  void ErrorNoOutput();

  /** Transmits new decoded metadata to the application */
  void NewMetaData();

 private:
  static GstEngine* instance() { return sInstance; }

  /**
   * Creates a GStreamer element and puts it into pipeline.
   * @param factoryName Name of the element class to create.
   * @param bin Container into which the element is put.
   * @param name Identifier for the element.
   * @return Pointer to the created element, or NULL for failure.
   */
  static GstElement* CreateElement(
      const QString& factoryName, GstElement* bin = 0, const QString& name = 0);

  // CALLBACKS:
  /** Bus message */
  //static GstBusSyncReply bus_cb( GstBus*, GstMessage*, gpointer );
  static gboolean BusCallback(GstBus*, GstMessage*, gpointer);
  /** Called when decodebin has generated a new pad */
  static void NewPadCallback(GstElement*, GstPad*, gboolean, gpointer);
  /** Used by canDecode(). When called, the format probably can be decoded */
  static void CanDecodeNewPadCallback(GstElement*, GstPad*, gboolean, gpointer);
  /** Used by canDecode(). Called after last pad so it makes no sense to wait anymore */
  static void CanDecodeLastCallback(GstElement*, gpointer);
  /** Called when new metadata tags have been found */
  static void EventCallback( GstPad*, GstEvent* event, gpointer arg);
  /** Duplicates audio data for application side processing */
  static void HandoffCallback( GstPad*, GstBuffer*, gpointer );

  /** Get a list of available plugins from a specified Class */
  PluginDetailsList GetPluginList(const QString& classname) const;

  /** Construct the output pipeline */
  bool CreatePipeline();

  /** Stops playback, destroys all input pipelines, destroys output pipeline, and frees resources */
  void DestroyPipeline();

  /* Constructs the pipeline for audio CDs, optionally selecting a device and/or track and/or setting the state to paused */
  bool SetupAudioCD( const QString& device, unsigned track, bool pause );

  /** Beams the streaming buffer status to Amarok */
  void SendBufferStatus();

  /////////////////////////////////////////////////////////////////////////////////////
  // DATA MEMBERS
  /////////////////////////////////////////////////////////////////////////////////////
  // Interval of main timer, handles the volume fading
  static const int kTimerInterval = 40; //msec
  static const int kGstStateTimeout = 10000000;

  QString sink_;

  static GstEngine* sInstance;

  GstElement* gst_pipeline_;

  GstElement* gst_src_;
  GstElement* gst_decodebin_;

  GstElement* gst_audiobin_;

  GstElement* gst_audioconvert_;
  //GstElement* gst_equalizer_;
  GstElement* gst_identity_;
  GstElement* gst_volume_;
  GstElement* gst_audioscale_;
  GstElement* gst_audiosink_;

  QString gst_error_;
  QString gst_debug_;

  int metacount_;

  uint event_cb_id_;

  //////////
  // scope
  //////////
  // delay queue for synchronizing samples to where the audio device is playing
  GQueue* delayq_;
  // the current set of samples for the scope, in case we don't have enough buffers yet
  // and end up with an incomplete buffer
  float current_scope_[SCOPESIZE];
  // the sample in m_currentScope we are working on
  int current_sample_;

  void UpdateScope();
  qint64 PruneScope();
  void ClearScopeQ();

  QMutex            scope_mutex_;

  bool              pipeline_filled_;
  float             fade_value_;

  bool              equalizer_enabled_;
  int               equalizer_preamp_;
  QList<int>        equalizer_gains_;

  Engine::SimpleMetaBundle meta_bundle_;

  bool shutdown_;
  mutable bool can_decode_success_;
  mutable bool can_decode_last_;

  GstElement* can_decode_pipeline_;
  GstElement* can_decode_src_;
  GstElement* can_decode_bin_;
};


#endif /*AMAROK_GSTENGINE_H*/

