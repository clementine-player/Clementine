#ifndef STEMMIXERWIDGET_H
#define STEMMIXERWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QSettings>

#include "engines/stemseparator.h"
#include "pythonstemcontroller.h"

// Forward declaration for original stem controller
class QProcess;

class QSpinBox;
class QCheckBox;

/**
 * @brief Qt Widget for controlling AI stem separation and mixing
 * 
 * This widget provides:
 * - Individual volume controls for each stem (vocals, drums, bass, other)
 * - Solo/Mute buttons per stem
 * - Enable/Disable stem separation
 * - Processing progress indication
 * - Real-time updates during playback
 */
class StemMixerWidget : public QWidget {
    Q_OBJECT

public:
    explicit StemMixerWidget(QWidget* parent = nullptr);
    ~StemMixerWidget();

    // Main control interface
    void setStemSeparator(StemSeparator* separator);
    void setCurrentTrack(const QString& audio_file);
    
    // Volume control (0-100)
    void setStemVolume(int stem_index, int volume);
    int getStemVolume(int stem_index) const;
    
    // Solo/Mute state
    void setStemSolo(int stem_index, bool solo);
    void setStemMute(int stem_index, bool mute);
    bool isStemSolo(int stem_index) const;
    bool isStemMute(int stem_index) const;
    
    // Query methods
    bool hasSeparatedStems() const { return current_stems_.is_valid; }
    bool isSeparationEnabled() const { return separation_enabled_; }
    bool isSeparationAvailable() const { return separation_available_; }
    
    // Master controls
    void setEnabled(bool enabled);
    void setSeparationAvailable(bool available);
    void reset();

public slots:
    void onSeparationStarted(const QString& job_id);
    void onSeparationProgress(const QString& job_id, int progress);
    void onSeparationFinished(const QString& job_id, const SeparatedStems& stems);
    void onSeparationError(const QString& job_id, const QString& error);
    // Stem engine integration (GstStemEngine) - for UI feedback
    void onStemEngineError(const QString& error_message);
    void onStemEngineStateChanged(int state); // 0=stopped,1=playing,2=paused

signals:
    void stemVolumeChanged(int stem_index, int volume);
    void stemSoloChanged(int stem_index, bool solo);
    void stemMuteChanged(int stem_index, bool mute);
    void separationEnabledChanged(bool enabled);
    void separationRequested(const QString& audio_file);
    void stemsReady(const SeparatedStems& stems);
    void playStems(int play_mode);
    void pauseStems();
    void stopStems();
    void resumeStems();
    void requestStopMainPlayer(); // New signal to stop main player
    void requestPauseMainPlayer(); // New signal to pause main player
    void originalStemReady();     // Signal when original stem system is ready
    void originalStemError(const QString& error); // Signal for original stem errors

private slots:
    void onVolumeSliderChanged(int value);
    void onSoloButtonClicked();
    void onMuteButtonClicked();
    void onEnableCheckboxChanged(bool checked);
    void onSeparateButtonClicked();
    void onLiveButtonClicked();         // New live button slot
    void onPlayPauseButtonClicked();    // New play/pause button slot
    void onStopButtonClicked();         // New stop button slot
    void onCancelButtonClicked();
    void onOriginalStemOutput();       // Handle original stem process output
    void onOriginalStemError();        // Handle original stem process errors
    void onOriginalStemFinished(int exitCode); // Handle process completion
    void updateUI();
    
    // Original stem implementation methods
    void startOriginalStemSeparation(const QString& audio_file);
    void stopOriginalStemSeparation();
    void sendOriginalStemCommand(const QJsonObject& command);

private:
    enum StemType {
        STEM_VOCALS = 0,
        STEM_DRUMS = 1,
        STEM_BASS = 2,
        STEM_OTHER = 3,
        STEM_COUNT = 4
    };

    struct StemControl {
        QLabel* label;
        QSlider* volume_slider;
        QSpinBox* volume_spinbox;
        QPushButton* solo_button;
        QPushButton* mute_button;
        QLabel* status_label;
        
        bool is_solo = false;
        bool is_mute = false;
        int volume = 100;
    };

    void setupUI();
    void setupStemControl(StemControl& control, const QString& name, const QColor& color);
    void connectSignals();
    void updateStemUI(int stem_index);
    void updateMasterUI();
    void updateModeIndicator();
    StemControl* getStemControl(QObject* sender);
    int getStemIndex(QObject* sender);
    
    // UI Components
    QVBoxLayout* main_layout_;
    QGroupBox* master_group_;
    QCheckBox* enable_checkbox_;
    QPushButton* separate_button_;
    QPushButton* live_button_;      // New live separation button
    QPushButton* play_pause_button_; // New play/pause button for stems
    QPushButton* stop_button_;       // New stop button for stems
    QPushButton* cancel_button_;
    QProgressBar* progress_bar_;
    QLabel* status_label_;
    QLabel* mode_label_; // Shows Live vs Offline mode
    
    QGroupBox* stems_group_;
    StemControl stem_controls_[STEM_COUNT];
    
    QGroupBox* presets_group_;
    QPushButton* karaoke_button_;
    QPushButton* drums_only_button_;
    QPushButton* instrumental_button_;
    QPushButton* reset_button_;
    
    // State management
    StemSeparator* stem_separator_;
    PythonStemController* python_controller_;
    QProcess* original_stem_process_;   // Process for original Python implementation
    QString current_track_;
    QString current_job_id_;
    SeparatedStems current_stems_;
    bool separation_enabled_;
    bool separation_available_;
    bool stem_playback_active_;     // New state tracking
    bool using_original_implementation_; // Flag for implementation mode
    bool original_live_ready_ = false; // LIVE mode ready (received READY)
        QString original_output_dir_;     // Output dir used by original live script (for pipes)
    struct UiStateSnapshot {
        bool has_separator=false;
        bool has_track=false;
        bool is_busy=false;
        bool separation_available=false;
        bool separation_enabled=false;
        bool stems_available=false;
        bool playback_active=false;
    } last_ui_state_;
    
    QTimer* ui_update_timer_;
    
    // Color scheme for stems
    static const QColor kStemColors[STEM_COUNT];
    static const QString kStemNames[STEM_COUNT];
};

#endif // STEMMIXERWIDGET_H
