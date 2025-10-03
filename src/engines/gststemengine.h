/* This file is part of Clementine.
   Copyright 2025, AI Stem Separation Implementation

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

#ifndef GSTSTEMENGINE_H
#define GSTSTEMENGINE_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <gst/gst.h>

#include "engines/stemseparator.h"

/**
 * @brief Real-time stem mixing engine using GStreamer
 * 
 * This class provides real-time mixing of separated stem files using GStreamer's
 * audiomixer element. It allows independent volume, mute, and solo control for
 * each stem while maintaining synchronized playback.
 */
class GstStemEngine : public QObject {
    Q_OBJECT

public:
    explicit GstStemEngine(QObject* parent = nullptr);
    ~GstStemEngine();

    // Stem management
    bool loadStems(const SeparatedStems& stems);
    void clearStems();
    bool hasStems() const { return stems_loaded_; }

    // Playback control
    bool play();
    bool pause();
    bool stop();
    bool seek(qint64 position_ms);
    qint64 position() const;
    qint64 duration() const;

    // Stem mixing controls
    void setStemVolume(int stem_index, float volume);    // 0.0 - 1.0
    void setStemMute(int stem_index, bool mute);
    void setStemSolo(int stem_index, bool solo);
    
    float getStemVolume(int stem_index) const;
    bool isStemMute(int stem_index) const;
    bool isStemSolo(int stem_index) const;

    // Output device
    void setOutputDevice(const QString& device);

signals:
    void positionChanged(qint64 position_ms);
    void durationChanged(qint64 duration_ms);
    void stateChanged(int state);  // 0=stopped, 1=playing, 2=paused
    void error(const QString& message);

private:
    // GStreamer pipeline setup
    bool createPipeline();
    void destroyPipeline();
    bool addStemSource(int stem_index, const QString& file_path);
    bool addLiveStemSource(int stem_index, const QString& pipe_path);  // New function for live pipes
    void updateMixerInputs();

    // Internal state
    bool stems_loaded_;
    SeparatedStems current_stems_;
    
    // GStreamer elements
    GstElement* pipeline_;
    GstElement* audiomixer_;
    GstElement* audioconvert_;
    GstElement* audioresample_;
    GstElement* autoaudiosink_;
    GstBus* bus_;
    
    // Stem sources (vocals, drums, bass, other)
    struct StemSource {
        GstElement* filesrc;
        GstElement* decodebin;
        GstElement* audioconvert;
        GstElement* audioresample;
        GstElement* volume;
        GstPad* mixer_pad;
        float volume_level;
        bool is_mute;
        bool is_solo;
    };
    
    StemSource stem_sources_[4];  // vocals=0, drums=1, bass=2, other=3
    
    // Mixing state
    bool any_solo_active_;
    QString output_device_;
    
    // Thread safety
    mutable QMutex mutex_;
    
    // Helper methods
    void applyStemState(int stem_index);
    void updateSoloState();
    static gboolean busCallback(GstBus* bus, GstMessage* message, gpointer user_data);
    static void padAddedCallback(GstElement* element, GstPad* pad, gpointer user_data);
};

#endif // GSTSTEMENGINE_H
