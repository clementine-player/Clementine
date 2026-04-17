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

#include "gststemengine.h"
#include "core/logging.h"

#include <QFileInfo>
#include <QMutexLocker>
#include <QTimer>

GstStemEngine::GstStemEngine(QObject* parent)
    : QObject(parent),
      stems_loaded_(false),
      pipeline_(nullptr),
      audiomixer_(nullptr),
      audioconvert_(nullptr),
      audioresample_(nullptr),
      autoaudiosink_(nullptr),
      bus_(nullptr),
      any_solo_active_(false) {
    
    // Initialize stem sources
    for (int i = 0; i < 4; ++i) {
        StemSource& source = stem_sources_[i];
        source.filesrc = nullptr;
        source.decodebin = nullptr;
        source.audioconvert = nullptr;
        source.audioresample = nullptr;
        source.volume = nullptr;
        source.mixer_pad = nullptr;
        source.volume_level = 1.0f;
        source.is_mute = false;
        source.is_solo = false;
    }
    
    qLog(Info) << "GstStemEngine created";
}

GstStemEngine::~GstStemEngine() {
    destroyPipeline();
    qLog(Info) << "GstStemEngine destroyed";
}

bool GstStemEngine::loadStems(const SeparatedStems& stems) {
    QMutexLocker locker(&mutex_);
    
    if (!stems.is_valid) {
        qLog(Warning) << "Cannot load invalid stems";
        return false;
    }
    
    // Stop current playback if active
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        destroyPipeline();
    }
    
    current_stems_ = stems;
    
    if (!createPipeline()) {
        qLog(Error) << "Failed to create GStreamer pipeline for stems";
        return false;
    }
    
    // Add all stem sources
    QStringList stem_files = {
        stems.vocals_file,   // 0
        stems.drums_file,    // 1  
        stems.bass_file,     // 2
        stems.other_file     // 3
    };
    
    // Check if we're in live mode (named pipes) or file mode
    bool is_live_mode = false;
    for (const QString& file : stem_files) {
        if (file.contains(".fifo") || file.contains("pipe")) {
            is_live_mode = true;
            break;
        }
    }
    
    if (is_live_mode) {
        qLog(Info) << "Loading stems in LIVE mode with named pipes";
        
        // For live mode, use different source elements
        for (int i = 0; i < 4; ++i) {
            if (!stem_files[i].isEmpty()) {
                if (!addLiveStemSource(i, stem_files[i])) {
                    qLog(Error) << "Failed to add live stem source" << i << ":" << stem_files[i];
                    destroyPipeline();
                    return false;
                }
            }
        }
        
        // Start pipeline immediately for live mode
        GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            qLog(Error) << "Failed to start live stem pipeline";
            destroyPipeline();
            return false;
        }
    } else {
        qLog(Info) << "Loading stems in FILE mode";
        
        // Regular file mode
        for (int i = 0; i < 4; ++i) {
            if (!addStemSource(i, stem_files[i])) {
                qLog(Error) << "Failed to add stem source" << i << ":" << stem_files[i];
                destroyPipeline();
                return false;
            }
        }
    }
    
    stems_loaded_ = true;
    qLog(Info) << "Successfully loaded stems for real-time mixing in" << (is_live_mode ? "LIVE" : "FILE") << "mode";
    
    return true;
}

void GstStemEngine::clearStems() {
    QMutexLocker locker(&mutex_);
    
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        destroyPipeline();
    }
    
    stems_loaded_ = false;
    current_stems_ = SeparatedStems();
    
    qLog(Info) << "Stems cleared";
}

bool GstStemEngine::createPipeline() {
    // Create main pipeline
    pipeline_ = gst_pipeline_new("stem-mixer-pipeline");
    if (!pipeline_) {
        qLog(Error) << "Failed to create pipeline";
        return false;
    }
    
    // Create mixer element
    audiomixer_ = gst_element_factory_make("audiomixer", "mixer");
    if (!audiomixer_) {
        qLog(Error) << "Failed to create audiomixer";
        return false;
    }
    
    // Create audio processing elements
    audioconvert_ = gst_element_factory_make("audioconvert", "convert");
    audioresample_ = gst_element_factory_make("audioresample", "resample");
    autoaudiosink_ = gst_element_factory_make("autoaudiosink", "sink");
    
    if (!audioconvert_ || !audioresample_ || !autoaudiosink_) {
        qLog(Error) << "Failed to create audio processing elements";
        return false;
    }
    
    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline_), 
                     audiomixer_, audioconvert_, audioresample_, autoaudiosink_, 
                     nullptr);
    
    // Link mixer output to sink
    if (!gst_element_link_many(audiomixer_, audioconvert_, audioresample_, autoaudiosink_, nullptr)) {
        qLog(Error) << "Failed to link mixer output elements";
        return false;
    }
    
    // Set up bus for messages
    bus_ = gst_element_get_bus(pipeline_);
    gst_bus_add_watch(bus_, busCallback, this);
    
    qLog(Info) << "GStreamer stem pipeline created successfully";
    return true;
}

void GstStemEngine::destroyPipeline() {
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline_));
        pipeline_ = nullptr;
    }
    
    if (bus_) {
        gst_object_unref(GST_OBJECT(bus_));
        bus_ = nullptr;
    }
    
    // Reset stem sources
    for (int i = 0; i < 4; ++i) {
        StemSource& source = stem_sources_[i];
        source.filesrc = nullptr;
        source.decodebin = nullptr;
        source.audioconvert = nullptr;
        source.audioresample = nullptr;
        source.volume = nullptr;
        source.mixer_pad = nullptr;
    }
    
    audiomixer_ = nullptr;
    audioconvert_ = nullptr;
    audioresample_ = nullptr;
    autoaudiosink_ = nullptr;
}

bool GstStemEngine::addStemSource(int stem_index, const QString& file_path) {
    if (stem_index < 0 || stem_index >= 4) {
        qLog(Error) << "Invalid stem index:" << stem_index;
        return false;
    }
    
    if (!QFileInfo::exists(file_path)) {
        qLog(Error) << "Stem file does not exist:" << file_path;
        return false;
    }
    
    StemSource& source = stem_sources_[stem_index];
    
    // Create source elements for this stem
    QString stem_name = QString("stem_%1").arg(stem_index);
    source.filesrc = gst_element_factory_make("filesrc", QString("%1_src").arg(stem_name).toUtf8().data());
    source.decodebin = gst_element_factory_make("decodebin", QString("%1_decode").arg(stem_name).toUtf8().data());
    source.audioconvert = gst_element_factory_make("audioconvert", QString("%1_convert").arg(stem_name).toUtf8().data());
    source.audioresample = gst_element_factory_make("audioresample", QString("%1_resample").arg(stem_name).toUtf8().data());
    source.volume = gst_element_factory_make("volume", QString("%1_volume").arg(stem_name).toUtf8().data());
    
    if (!source.filesrc || !source.decodebin || !source.audioconvert || 
        !source.audioresample || !source.volume) {
        qLog(Error) << "Failed to create elements for stem" << stem_index;
        return false;
    }
    
    // Set file location
    g_object_set(G_OBJECT(source.filesrc), "location", file_path.toUtf8().data(), nullptr);
    
    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline_), 
                     source.filesrc, source.decodebin, source.audioconvert,
                     source.audioresample, source.volume,
                     nullptr);
    
    // Link source chain (except decodebin - that needs pad-added callback)
    if (!gst_element_link(source.filesrc, source.decodebin)) {
        qLog(Error) << "Failed to link filesrc to decodebin for stem" << stem_index;
        return false;
    }
    
    // Link processing chain
    if (!gst_element_link_many(source.audioconvert, source.audioresample, source.volume, nullptr)) {
        qLog(Error) << "Failed to link processing chain for stem" << stem_index;
        return false;
    }
    
    // Connect decodebin pad-added signal for dynamic linking
    g_signal_connect(source.decodebin, "pad-added", G_CALLBACK(padAddedCallback), this);
    
    // Request mixer pad and link volume output to mixer
    GstPadTemplate* mixer_pad_template = gst_element_class_get_pad_template(
        GST_ELEMENT_GET_CLASS(audiomixer_), "sink_%u");
    source.mixer_pad = gst_element_request_pad(audiomixer_, mixer_pad_template, nullptr, nullptr);
    
    GstPad* volume_src_pad = gst_element_get_static_pad(source.volume, "src");
    if (gst_pad_link(volume_src_pad, source.mixer_pad) != GST_PAD_LINK_OK) {
        qLog(Error) << "Failed to link volume to mixer for stem" << stem_index;
        gst_object_unref(volume_src_pad);
        return false;
    }
    gst_object_unref(volume_src_pad);
    
    qLog(Info) << "Added stem source" << stem_index << ":" << file_path;
    return true;
}

bool GstStemEngine::addLiveStemSource(int stem_index, const QString& pipe_path) {
    if (stem_index < 0 || stem_index >= 4) {
        qLog(Error) << "Invalid stem index:" << stem_index;
        return false;
    }
    
    if (pipe_path.isEmpty()) {
        qLog(Warning) << "Empty pipe path for stem" << stem_index;
        return false;
    }
    
    StemSource& source = stem_sources_[stem_index];
    
    // Create source elements for live streaming (filesrc works fine with named pipes)
    QString stem_name = QString("stem_%1").arg(stem_index);
    source.filesrc = gst_element_factory_make("filesrc", QString("%1_src").arg(stem_name).toUtf8().data());
    source.decodebin = gst_element_factory_make("decodebin", QString("%1_decode").arg(stem_name).toUtf8().data());
    source.audioconvert = gst_element_factory_make("audioconvert", QString("%1_convert").arg(stem_name).toUtf8().data());
    source.audioresample = gst_element_factory_make("audioresample", QString("%1_resample").arg(stem_name).toUtf8().data());
    source.volume = gst_element_factory_make("volume", QString("%1_volume").arg(stem_name).toUtf8().data());
    
    if (!source.filesrc || !source.decodebin || !source.audioconvert || 
        !source.audioresample || !source.volume) {
        qLog(Error) << "Failed to create live elements for stem" << stem_index;
        return false;
    }
    
    // Set pipe location
    g_object_set(G_OBJECT(source.filesrc), "location", pipe_path.toUtf8().data(), nullptr);
    
    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline_), 
                     source.filesrc, source.decodebin, source.audioconvert,
                     source.audioresample, source.volume,
                     nullptr);
    
    // Link the chain: filesrc -> decodebin -> (dynamic pad callback) -> audioconvert -> audioresample -> volume
    if (!gst_element_link(source.filesrc, source.decodebin)) {
        qLog(Error) << "Failed to link filesrc to decodebin for live stem" << stem_index;
        return false;
    }
    
    // Connect the decodebin pad-added signal for dynamic linking
    g_signal_connect(source.decodebin, "pad-added", G_CALLBACK(padAddedCallback), this);
    
    // Link static chain: audioconvert -> audioresample -> volume
    if (!gst_element_link_many(source.audioconvert, source.audioresample, source.volume, nullptr)) {
        qLog(Error) << "Failed to link live processing chain for stem" << stem_index;
        return false;
    }
    
    // Request mixer pad and link volume output to mixer
    GstPadTemplate* mixer_pad_template = gst_element_class_get_pad_template(
        GST_ELEMENT_GET_CLASS(audiomixer_), "sink_%u");
    source.mixer_pad = gst_element_request_pad(audiomixer_, mixer_pad_template, nullptr, nullptr);
    
    GstPad* volume_src_pad = gst_element_get_static_pad(source.volume, "src");
    if (gst_pad_link(volume_src_pad, source.mixer_pad) != GST_PAD_LINK_OK) {
        qLog(Error) << "Failed to link volume to mixer for live stem" << stem_index;
        gst_object_unref(volume_src_pad);
        return false;
    }
    gst_object_unref(volume_src_pad);
    
    qLog(Info) << "Added LIVE stem source" << stem_index << ":" << pipe_path;
    return true;
}

bool GstStemEngine::play() {
    QMutexLocker locker(&mutex_);
    
    if (!pipeline_ || !stems_loaded_) {
        qLog(Warning) << "Cannot play: no stems loaded";
        return false;
    }
    
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qLog(Error) << "Failed to start stem playback";
        return false;
    }
    
    qLog(Info) << "Started stem playback";
    emit stateChanged(1); // playing
    return true;
}

bool GstStemEngine::pause() {
    QMutexLocker locker(&mutex_);
    
    if (!pipeline_) {
        return false;
    }
    
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qLog(Error) << "Failed to pause stem playback";
        return false;
    }
    
    qLog(Info) << "Paused stem playback";
    emit stateChanged(2); // paused
    return true;
}

bool GstStemEngine::stop() {
    QMutexLocker locker(&mutex_);
    
    if (!pipeline_) {
        return false;
    }
    
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qLog(Error) << "Failed to stop stem playback";
        return false;
    }
    
    qLog(Info) << "Stopped stem playback";
    emit stateChanged(0); // stopped
    return true;
}

void GstStemEngine::setStemVolume(int stem_index, float volume) {
    QMutexLocker locker(&mutex_);
    
    if (stem_index < 0 || stem_index >= 4) {
        return;
    }
    
    volume = qBound(0.0f, volume, 1.0f);
    stem_sources_[stem_index].volume_level = volume;
    
    applyStemState(stem_index);
    
    qLog(Debug) << "Set stem" << stem_index << "volume to" << volume;
}

void GstStemEngine::setStemMute(int stem_index, bool mute) {
    QMutexLocker locker(&mutex_);
    
    if (stem_index < 0 || stem_index >= 4) {
        return;
    }
    
    stem_sources_[stem_index].is_mute = mute;
    applyStemState(stem_index);
    
    qLog(Debug) << "Set stem" << stem_index << "mute to" << mute;
}

void GstStemEngine::setStemSolo(int stem_index, bool solo) {
    QMutexLocker locker(&mutex_);
    
    if (stem_index < 0 || stem_index >= 4) {
        return;
    }
    
    stem_sources_[stem_index].is_solo = solo;
    updateSoloState();
    
    qLog(Debug) << "Set stem" << stem_index << "solo to" << solo;
}

void GstStemEngine::applyStemState(int stem_index) {
    StemSource& source = stem_sources_[stem_index];
    
    if (!source.volume) {
        return;
    }
    
    // Calculate effective volume
    float effective_volume = 0.0f;
    
    if (any_solo_active_) {
        // If any stem is solo, only solo stems should be audible
        if (source.is_solo && !source.is_mute) {
            effective_volume = source.volume_level;
        }
    } else {
        // Normal mode: mute overrides volume
        if (!source.is_mute) {
            effective_volume = source.volume_level;
        }
    }
    
    g_object_set(G_OBJECT(source.volume), "volume", (gdouble)effective_volume, nullptr);
}

void GstStemEngine::updateSoloState() {
    // Check if any stem is in solo mode
    any_solo_active_ = false;
    for (int i = 0; i < 4; ++i) {
        if (stem_sources_[i].is_solo) {
            any_solo_active_ = true;
            break;
        }
    }
    
    // Apply state to all stems
    for (int i = 0; i < 4; ++i) {
        applyStemState(i);
    }
}

float GstStemEngine::getStemVolume(int stem_index) const {
    QMutexLocker locker(&mutex_);
    
    if (stem_index < 0 || stem_index >= 4) {
        return 0.0f;
    }
    
    return stem_sources_[stem_index].volume_level;
}

bool GstStemEngine::isStemMute(int stem_index) const {
    QMutexLocker locker(&mutex_);
    
    if (stem_index < 0 || stem_index >= 4) {
        return true;
    }
    
    return stem_sources_[stem_index].is_mute;
}

bool GstStemEngine::isStemSolo(int stem_index) const {
    QMutexLocker locker(&mutex_);
    
    if (stem_index < 0 || stem_index >= 4) {
        return false;
    }
    
    return stem_sources_[stem_index].is_solo;
}

gboolean GstStemEngine::busCallback(GstBus* bus, GstMessage* message, gpointer user_data) {
    Q_UNUSED(bus)
    GstStemEngine* engine = static_cast<GstStemEngine*>(user_data);
    
    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError* error = nullptr;
            gchar* debug = nullptr;
            gst_message_parse_error(message, &error, &debug);
            
            QString error_msg = QString("GStreamer error: %1").arg(error->message);
            if (debug) {
                error_msg += QString(" Debug: %1").arg(debug);
            }
            
            qLog(Error) << error_msg;
            emit engine->error(error_msg);
            
            g_error_free(error);
            g_free(debug);
            break;
        }
        
        case GST_MESSAGE_EOS:
            qLog(Info) << "End of stream reached";
            emit engine->stateChanged(0); // stopped
            break;
            
        case GST_MESSAGE_STATE_CHANGED: {
            if (GST_MESSAGE_SRC(message) == GST_OBJECT(engine->pipeline_)) {
                GstState old_state, new_state;
                gst_message_parse_state_changed(message, &old_state, &new_state, nullptr);
                
                qLog(Debug) << "Pipeline state changed from" << gst_element_state_get_name(old_state)
                           << "to" << gst_element_state_get_name(new_state);
            }
            break;
        }
        
        default:
            break;
    }
    
    return TRUE;
}

void GstStemEngine::padAddedCallback(GstElement* element, GstPad* pad, gpointer user_data) {
    Q_UNUSED(element)
    GstStemEngine* engine = static_cast<GstStemEngine*>(user_data);
    
    // Find which stem this decodebin belongs to
    for (int i = 0; i < 4; ++i) {
        if (element == engine->stem_sources_[i].decodebin) {
            // Link to audioconvert
            GstPad* convert_sink_pad = gst_element_get_static_pad(engine->stem_sources_[i].audioconvert, "sink");
            if (gst_pad_link(pad, convert_sink_pad) != GST_PAD_LINK_OK) {
                qLog(Error) << "Failed to link decodebin to audioconvert for stem" << i;
            } else {
                qLog(Debug) << "Successfully linked decodebin pad for stem" << i;
            }
            gst_object_unref(convert_sink_pad);
            break;
        }
    }
}

qint64 GstStemEngine::position() const {
    QMutexLocker locker(&mutex_);
    
    if (!pipeline_) {
        return 0;
    }
    
    gint64 pos = 0;
    if (gst_element_query_position(pipeline_, GST_FORMAT_TIME, &pos)) {
        return pos / GST_MSECOND;
    }
    
    return 0;
}

qint64 GstStemEngine::duration() const {
    QMutexLocker locker(&mutex_);
    
    if (!pipeline_) {
        return 0;
    }
    
    gint64 dur = 0;
    if (gst_element_query_duration(pipeline_, GST_FORMAT_TIME, &dur)) {
        return dur / GST_MSECOND;
    }
    
    return 0;
}

bool GstStemEngine::seek(qint64 position_ms) {
    QMutexLocker locker(&mutex_);
    
    if (!pipeline_) {
        return false;
    }
    
    gint64 pos_ns = position_ms * GST_MSECOND;
    
    return gst_element_seek_simple(pipeline_, GST_FORMAT_TIME, 
                                  GST_SEEK_FLAG_FLUSH, pos_ns);
}

void GstStemEngine::setOutputDevice(const QString& device) {
    QMutexLocker locker(&mutex_);
    output_device_ = device;
    
    // TODO: Apply device change to autoaudiosink if needed
    qLog(Info) << "Set output device to:" << device;
}
