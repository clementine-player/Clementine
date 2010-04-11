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
#include <boost/shared_ptr.hpp>

class QTimerEvent;

class GstEnginePipeline;

/**
 * @class GstEngine
 * @short GStreamer engine plugin
 * @author Mark Kretschmann <markey@web.de>
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

  void gstStatusText(const QString& str) { emit statusText( str ); }
  void gstMetaData(Engine::SimpleMetaBundle &bundle) { emit metaData( bundle ); }

  PluginDetailsList GetOutputsList() const { return GetPluginList( "Sink/Audio" ); }
  static bool DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(const QString& name);

  static GstElement* CreateElement(
      const QString& factoryName, GstElement* bin = 0, const QString& name = 0);

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
  void EndOfStreamReached();
  void HandlePipelineError(const QString& message);
  void NewMetaData(const Engine::SimpleMetaBundle& bundle);
  void NewBuffer(GstBuffer* buf);
  void ClearScopeQ();
  void FadeoutFinished();

 private:
  // Callbacks
  static void CanDecodeNewPadCallback(GstElement*, GstPad*, gboolean, gpointer);
  static void CanDecodeLastCallback(GstElement*, gpointer);

  /** Get a list of available plugins from a specified Class */
  PluginDetailsList GetPluginList(const QString& classname) const;

  /** Construct the output pipeline */
  boost::shared_ptr<GstEnginePipeline> CreatePipeline(const QUrl& url);

  /** Beams the streaming buffer status to Amarok */
  void SendBufferStatus();

  void StartFadeout();

  /////////////////////////////////////////////////////////////////////////////////////
  // DATA MEMBERS
  /////////////////////////////////////////////////////////////////////////////////////
  // Interval of main timer, handles the volume fading
  static const int kTimerInterval = 40; //msec

  QString sink_;
  QString device_;

  boost::shared_ptr<GstEnginePipeline> current_pipeline_;
  boost::shared_ptr<GstEnginePipeline> fadeout_pipeline_;

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

  QMutex            scope_mutex_;

  float             fade_value_;

  bool              equalizer_enabled_;
  int               equalizer_preamp_;
  QList<int>        equalizer_gains_;

  bool shutdown_;
  mutable bool can_decode_success_;
  mutable bool can_decode_last_;

  GstElement* can_decode_pipeline_;
  GstElement* can_decode_src_;
  GstElement* can_decode_bin_;
};


#endif /*AMAROK_GSTENGINE_H*/

