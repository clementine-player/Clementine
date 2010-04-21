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

  bool Init();

  bool CanDecode(const QUrl& url);

  uint position() const;
  uint length() const;
  Engine::State state() const;
  const Engine::Scope& scope();

  PluginDetailsList GetOutputsList() const { return GetPluginList( "Sink/Audio" ); }
  static bool DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(const QString& name);

  GstElement* CreateElement(
      const QString& factoryName, GstElement* bin = 0, const QString& name = 0);

 public slots:
  bool Load(const QUrl&, Engine::TrackChangeType change);
  bool Play(uint offset);
  void Stop();
  void Pause();
  void Unpause();
  void Seek(uint ms);

  /** Set whether equalizer is enabled */
  void SetEqualizerEnabled(bool);

  /** Set equalizer preamp and gains, range -100..100. Gains are 10 values. */
  void SetEqualizerParameters(int preamp, const QList<int>& bandGains);

  void ReloadSettings();

 protected:
  void SetVolumeSW(uint percent);
  void timerEvent(QTimerEvent*);

 private slots:
  void EndOfStreamReached();
  void HandlePipelineError(const QString& message);
  void NewMetaData(const Engine::SimpleMetaBundle& bundle);
  void AddBufferToScope(GstBuffer* buf);
  void ClearScopeBuffers();
  void FadeoutFinished();

 private:
  // Callbacks
  static void CanDecodeNewPadCallback(GstElement*, GstPad*, gboolean, gpointer);
  static void CanDecodeLastCallback(GstElement*, gpointer);

  PluginDetailsList GetPluginList(const QString& classname) const;

  void StartFadeout();

  boost::shared_ptr<GstEnginePipeline> CreatePipeline(const QUrl& url);

  void UpdateScope();
  qint64 PruneScope();

 private:
  // Interval of main timer, handles the volume fading
  static const int kTimerInterval = 40; //msec

  QString sink_;
  QString device_;

  boost::shared_ptr<GstEnginePipeline> current_pipeline_;
  boost::shared_ptr<GstEnginePipeline> fadeout_pipeline_;

  GQueue* delayq_;
  float current_scope_[kScopeSize];
  int current_sample_;

  bool equalizer_enabled_;
  int equalizer_preamp_;
  QList<int> equalizer_gains_;

  mutable bool can_decode_success_;
  mutable bool can_decode_last_;
};


#endif /*AMAROK_GSTENGINE_H*/

