/***************************************************************************
 *   Copyright (C) 2004,5 Max Howell <max.howell@methylblue.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef XINE_ENGINE_H
#define XINE_ENGINE_H

#include "enginebase.h"
#include <QThread>
#include <QEvent>
#include <QSettings>
#include <QMutex>

extern "C"
{
    #include <sys/types.h>
    #include <xine.h>
}

class XineConfigDialog;

class XineEvent : public QEvent {
public:
  enum EventType {
    PlaybackFinished = QEvent::User + 1,
    InfoMessage,
    StatusMessage,
    MetaInfoChanged,
    Redirecting,
    LastFMTrackChanged,
  };

  XineEvent(EventType type, void* data = NULL) : QEvent(QEvent::Type(type)), data_(data) {}

  void setData(void* data) { data_ = data; }
  void* data() const { return data_; }

private:
  void* data_;
};

class PruneScopeThread;

class XineEngine : public Engine::Base
{
    Q_OBJECT

    friend class Fader;
    friend class OutFader;
    friend class PruneScopeThread;

   ~XineEngine();

    virtual bool Init();
    virtual bool CanDecode( const QUrl& );
    virtual bool Load( const QUrl &url, Engine::TrackChangeType change );
    virtual bool Play( uint = 0 );
    virtual void Stop();
    virtual void Pause();
    virtual void Unpause();
    virtual uint position() const;
    virtual uint length() const;
    virtual void Seek( uint );

    virtual bool metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle &b);
    virtual bool getAudioCDContents(const QString &device, QList<QUrl> &urls);
    virtual bool flushBuffer();

    virtual Engine::State state() const;
    virtual const Engine::Scope &scope();

    virtual void setEqualizerEnabled( bool );
    virtual void setEqualizerParameters( int preamp, const QList<int>& );
    virtual void SetVolumeSW( uint );
    virtual void fadeOut( uint fadeLength, bool* terminate, bool exiting = false );

    static  void XineEventListener( void*, const xine_event_t* );
    virtual bool event( QEvent* );

    virtual void playlistChanged();
    virtual void reloadSettings();

    Engine::SimpleMetaBundle fetchMetaData() const;

    virtual bool lastFmProxyRequired();

    bool makeNewStream();
    bool ensureStream();

    void determineAndShowErrorMessage(); //call after failure to load/play

    xine_t             *xine_;
    xine_stream_t      *stream_;
    xine_audio_port_t  *audioPort_;
    xine_event_queue_t *eventQueue_;
    xine_post_t        *post_;

    int64_t             currentVpts_;
    float               preamp_;

    bool                stopFader_;
    bool                fadeOutRunning_;

    QString             currentAudioPlugin_; //to see if audio plugin has been changed
    //need to save these for when the audio plugin is changed and xine reloaded
    bool                equalizerEnabled_;
    int                 intPreamp_;
    QList<int>     equalizerGains_;

    QMutex initMutex_;

    QSettings settings_;
    bool fadeoutOnExit_;
    bool fadeoutEnabled_;
    bool crossfadeEnabled_;
    int fadeoutDuration_;
    int xfadeLength_;
    bool xfadeNextTrack_;
    QUrl url_;

    PruneScopeThread* prune_;

    mutable Engine::SimpleMetaBundle currentBundle_;

public:
    XineEngine();

  private slots:
    void PruneScope();

signals:
    void resetConfig(xine_t *xine);
    void InfoMessage(const QString&);
    void LastFmTrackChange();
};

class Fader : public QThread
{
    XineEngine         *engine_;
    xine_t             *xine_;
    xine_stream_t      *decrease_;
    xine_stream_t      *increase_;
    xine_audio_port_t  *port_;
    xine_post_t        *post_;
    uint               fadeLength_;
    bool               paused_;
    bool               terminated_;

    virtual void run();

public:
    Fader( XineEngine *, uint fadeLengthMs );
   ~Fader();
   void pause();
   void resume();
   void finish();
};

class OutFader : public QThread
{
    XineEngine *engine_;
    bool        terminated_;
    uint        fadeLength_;

    virtual void run();

public:
    OutFader( XineEngine *, uint fadeLengthMs );
    ~OutFader();

   void finish();
};

class PruneScopeThread : public QThread {
public:
  PruneScopeThread(XineEngine* parent);

protected:
  virtual void run();

private:
  XineEngine* engine_;
};

#endif
