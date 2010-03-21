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

    virtual bool init();
    virtual bool canDecode( const QUrl& ) const;
    virtual bool load( const QUrl &url, bool stream );
    virtual bool play( uint = 0 );
    virtual void stop();
    virtual void pause();
    virtual void unpause();
    virtual uint position() const;
    virtual uint length() const;
    virtual void seek( uint );

    virtual bool metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle &b);
    virtual bool getAudioCDContents(const QString &device, QList<QUrl> &urls);
    virtual bool flushBuffer();

    virtual Engine::State state() const;
    virtual const Engine::Scope &scope();

    virtual void setEqualizerEnabled( bool );
    virtual void setEqualizerParameters( int preamp, const QList<int>& );
    virtual void setVolumeSW( uint );
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

    xine_t             *m_xine;
    xine_stream_t      *m_stream;
    xine_audio_port_t  *m_audioPort;
    xine_event_queue_t *m_eventQueue;
    xine_post_t        *m_post;

    int64_t             m_currentVpts;
    float               m_preamp;

    bool                m_stopFader;
    bool                m_fadeOutRunning;

    QString             m_currentAudioPlugin; //to see if audio plugin has been changed
    //need to save these for when the audio plugin is changed and xine reloaded
    bool                m_equalizerEnabled;
    int                 m_intPreamp;
    QList<int>     m_equalizerGains;

    QMutex m_initMutex;

    QSettings m_settings;
    bool m_fadeoutOnExit;
    bool m_fadeoutEnabled;
    bool m_crossfadeEnabled;
    int m_fadeoutDuration;

    PruneScopeThread* prune_;

    mutable Engine::SimpleMetaBundle m_currentBundle;

public:
    XineEngine();

  private slots:
    void PruneScope();

signals:
    void resetConfig(xine_t *xine);
};

class Fader : public QThread
{
    XineEngine         *m_engine;
    xine_t             *m_xine;
    xine_stream_t      *m_decrease;
    xine_stream_t      *m_increase;
    xine_audio_port_t  *m_port;
    xine_post_t        *m_post;
    uint               m_fadeLength;
    bool               m_paused;
    bool               m_terminated;

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
    XineEngine *m_engine;
    bool        m_terminated;
    uint        m_fadeLength;

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
