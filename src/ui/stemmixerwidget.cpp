#include "stemmixerwidget.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QStyle>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFile>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>

#include "core/logging.h"
#include "engines/stemseparator.h"

// Static member definitions
const QColor StemMixerWidget::kStemColors[StemMixerWidget::STEM_COUNT] = {
    QColor(255, 100, 100),  // Vocals - red
    QColor(100, 255, 100),  // Drums - green
    QColor(100, 100, 255),  // Bass - blue
    QColor(255, 255, 100)   // Other - yellow
};

const QString StemMixerWidget::kStemNames[StemMixerWidget::STEM_COUNT] = {
    "Vocals",
    "Drums", 
    "Bass",
    "Other"
};

namespace {
    const int STEM_COUNT = 4;
    const QStringList kStemNames = {"Vocals", "Drums", "Bass", "Other"};
    const QList<QColor> kStemColors = {
        QColor("#ff6b6b"), // Red for vocals
        QColor("#4ecdc4"), // Teal for drums  
        QColor("#45b7d1"), // Blue for bass
        QColor("#96ceb4")  // Green for other
    };
}

StemMixerWidget::StemMixerWidget(QWidget* parent)
    : QWidget(parent),
      stem_separator_(nullptr),
      python_controller_(nullptr),
      original_stem_process_(nullptr),
      separation_enabled_(false),
      separation_available_(false),
      stem_playback_active_(false),
    using_original_implementation_(false), // Start in OFFLINE mode; live mode deferred
      ui_update_timer_(new QTimer(this)) {
    
    // Create Python controller
    python_controller_ = new PythonStemController(this);
    
    // Connect Python controller signals
    connect(python_controller_, &PythonStemController::playerReady,
            this, [this]() {
                qDebug() << "Python stem player ready";
                stem_playback_active_ = false;
                updateMasterUI();
            });
    
    connect(python_controller_, &PythonStemController::playerError,
            this, [this](const QString& error) {
                qCritical() << "Python stem player error:" << error;
                stem_playback_active_ = false;
                updateMasterUI();
            });
    
    connect(python_controller_, &PythonStemController::playbackStateChanged,
            this, [this](PythonStemController::PlaybackState state) {
                stem_playback_active_ = (state == PythonStemController::PLAYING);
                updateMasterUI();
            });
    
    connect(python_controller_, &PythonStemController::progressChanged,
            this, [this](float progress) {
                progress_bar_->setValue(static_cast<int>(progress * 100));
            });
    
    setupUI();

    // Load persisted stem control settings
    QSettings settings("Clementine","StemMixer");
    settings.beginGroup("controls");
    for (int i=0;i<STEM_COUNT;++i) {
        int vol = settings.value(QString::number(i)+"/volume",100).toInt();
        bool mute = settings.value(QString::number(i)+"/mute",false).toBool();
        bool solo = settings.value(QString::number(i)+"/solo",false).toBool();
        stem_controls_[i].volume_slider->setValue(vol);
        stem_controls_[i].volume_spinbox->setValue(vol);
        stem_controls_[i].volume = vol;
        stem_controls_[i].mute_button->setChecked(mute);
        stem_controls_[i].is_mute = mute;
        stem_controls_[i].solo_button->setChecked(solo);
        stem_controls_[i].is_solo = solo;
        updateStemUI(i);
    }
    settings.endGroup();
    
    // Update UI every 100ms
    ui_update_timer_->setInterval(100);
    connect(ui_update_timer_, &QTimer::timeout, this, &StemMixerWidget::updateUI);
    ui_update_timer_->start();
}

StemMixerWidget::~StemMixerWidget() {
    if (ui_update_timer_) {
        ui_update_timer_->stop();
    }
    
    // Clean up original stem process
    if (original_stem_process_) {
        original_stem_process_->terminate();
        if (!original_stem_process_->waitForFinished(3000)) {
            original_stem_process_->kill();
        }
        original_stem_process_ = nullptr;
    }
}

void StemMixerWidget::setupUI() {
    setWindowTitle("🎛️ AI Stem Mixer");
    setMinimumSize(300, 500);
    
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(10);
    main_layout->setContentsMargins(10, 10, 10, 10);
    
    // Title and status
    QLabel* title_label = new QLabel("🎛️ AI Stem Mixer");
    title_label->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
    title_label->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(title_label);
    
    status_label_ = new QLabel("No track loaded");
    status_label_->setStyleSheet("color: #666; font-style: italic;");
    status_label_->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(status_label_);
    // Mode indicator label (Live vs Offline vs none)
    mode_label_ = new QLabel("Mode: -");
    mode_label_->setStyleSheet("color:#555; font-size:11px;");
    mode_label_->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(mode_label_);
    
    // Master controls group
    QGroupBox* master_group = new QGroupBox("Master Controls");
    QVBoxLayout* master_layout = new QVBoxLayout(master_group);
    
    // Enable checkbox
    enable_checkbox_ = new QCheckBox("Enable AI Stem Separation");
    enable_checkbox_->setEnabled(false);
    master_layout->addWidget(enable_checkbox_);
    
    // Separate button
    separate_button_ = new QPushButton("🤖 Separate Current Track");
    separate_button_->setEnabled(false);
    separate_button_->setMinimumHeight(35);
    separate_button_->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #45a049;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "}"
    );
    master_layout->addWidget(separate_button_);
    
    // Live separation button (currently hidden; feature deferred)
    live_button_ = new QPushButton("🔴 Start Live Separation");
    live_button_->setEnabled(false);
    live_button_->setMinimumHeight(35);
    live_button_->setStyleSheet(
        "QPushButton {"
        "  background-color: #FF5722;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E64A19;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "}"
    );
    master_layout->addWidget(live_button_);
    live_button_->hide(); // Hide until live mode is resumed later
    
    // Playback control buttons
    QHBoxLayout* playback_layout = new QHBoxLayout();
    
    // Play/Pause button
    play_pause_button_ = new QPushButton("▶️ Play Stems");
    play_pause_button_->setEnabled(false);
    play_pause_button_->setMinimumHeight(35);
    play_pause_button_->setStyleSheet(
        "QPushButton {"
        "  background-color: #2196F3;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #1976D2;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "}"
    );
    playback_layout->addWidget(play_pause_button_);
    
    // Stop button
    stop_button_ = new QPushButton("⏹️ Stop");
    stop_button_->setEnabled(false);
    stop_button_->setMinimumHeight(35);
    stop_button_->setStyleSheet(
        "QPushButton {"
        "  background-color: #FF9800;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #F57C00;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "}"
    );
    playback_layout->addWidget(stop_button_);
    
    master_layout->addLayout(playback_layout);
    
    // Cancel button
    cancel_button_ = new QPushButton("❌ Cancel Separation");
    cancel_button_->setEnabled(false);
    cancel_button_->setVisible(false);
    cancel_button_->setMinimumHeight(35);
    cancel_button_->setStyleSheet(
        "QPushButton {"
        "  background-color: #f44336;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #da190b;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "}"
    );
    master_layout->addWidget(cancel_button_);
    
    // Progress bar
    progress_bar_ = new QProgressBar();
    progress_bar_->setVisible(false);
    progress_bar_->setStyleSheet(
        "QProgressBar {"
        "  border: 1px solid #ccc;"
        "  border-radius: 4px;"
        "  text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #4CAF50;"
        "  border-radius: 3px;"
        "}"
    );
    master_layout->addWidget(progress_bar_);
    
    main_layout->addWidget(master_group);
    
    // Individual stem controls
    stems_group_ = new QGroupBox("Individual Stem Controls");
    stems_group_->setEnabled(false);
    QVBoxLayout* stems_layout = new QVBoxLayout(stems_group_);
    
    for (int i = 0; i < STEM_COUNT; ++i) {
        setupStemControl(stem_controls_[i], kStemNames[i], kStemColors[i]);
        
        QWidget* stem_widget = new QWidget();
        QHBoxLayout* stem_layout = new QHBoxLayout(stem_widget);
        stem_layout->setContentsMargins(0, 0, 0, 0);
        
        stem_layout->addWidget(stem_controls_[i].label);
        stem_layout->addWidget(stem_controls_[i].volume_slider);
        stem_layout->addWidget(stem_controls_[i].volume_spinbox);
        stem_layout->addWidget(stem_controls_[i].solo_button);
        stem_layout->addWidget(stem_controls_[i].mute_button);
        stem_layout->addWidget(stem_controls_[i].status_label);
        
        stems_layout->addWidget(stem_widget);
    }
    
    main_layout->addWidget(stems_group_);
    
    // Preset controls
    presets_group_ = new QGroupBox("Quick Presets");
    presets_group_->setEnabled(false);
    QHBoxLayout* presets_layout = new QHBoxLayout(presets_group_);
    
    QPushButton* vocals_only = new QPushButton("Vocals Only");
    QPushButton* instrumental = new QPushButton("Instrumental");
    QPushButton* drums_only = new QPushButton("Drums Only");
    QPushButton* bass_only = new QPushButton("Bass Only");
    QPushButton* other_only = new QPushButton("Other Only");
    QPushButton* reset_button = new QPushButton("Reset All");
    
    presets_layout->addWidget(vocals_only);
    presets_layout->addWidget(instrumental);
    presets_layout->addWidget(drums_only);
    presets_layout->addWidget(bass_only);
    presets_layout->addWidget(other_only);
    presets_layout->addWidget(reset_button);
    
    main_layout->addWidget(presets_group_);
    main_layout->addStretch();
    
    // Connect signals
    connect(enable_checkbox_, &QCheckBox::toggled, 
            this, &StemMixerWidget::onEnableCheckboxChanged);
    connect(separate_button_, &QPushButton::clicked,
            this, &StemMixerWidget::onSeparateButtonClicked);
    connect(live_button_, &QPushButton::clicked,
            this, &StemMixerWidget::onLiveButtonClicked);
    connect(play_pause_button_, &QPushButton::clicked,
            this, &StemMixerWidget::onPlayPauseButtonClicked);
    connect(stop_button_, &QPushButton::clicked,
            this, &StemMixerWidget::onStopButtonClicked);
    connect(cancel_button_, &QPushButton::clicked,
            this, &StemMixerWidget::onCancelButtonClicked);
    
    // Stem controls
    for (int i = 0; i < STEM_COUNT; ++i) {
        StemControl& control = stem_controls_[i];
        
        connect(control.volume_slider, &QSlider::valueChanged,
                this, &StemMixerWidget::onVolumeSliderChanged);
        connect(control.volume_spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &StemMixerWidget::onVolumeSliderChanged);
        connect(control.solo_button, &QPushButton::clicked,
                this, &StemMixerWidget::onSoloButtonClicked);
        connect(control.mute_button, &QPushButton::clicked,
                this, &StemMixerWidget::onMuteButtonClicked);
    }
    
    // Preset buttons
    connect(vocals_only, &QPushButton::clicked, [this]() {
        setStemMute(0, false); setStemMute(1, true); setStemMute(2, true); setStemMute(3, true);
        if (separation_available_ && current_stems_.is_valid) {
            emit playStems(1); // VocalsOnly = 1
        }
    });
    connect(instrumental, &QPushButton::clicked, [this]() {
        setStemMute(0, true); setStemMute(1, false); setStemMute(2, false); setStemMute(3, false);
        if (separation_available_ && current_stems_.is_valid) {
            emit playStems(2); // Instrumental = 2
        }
    });
    connect(drums_only, &QPushButton::clicked, [this]() {
        setStemMute(0, true); setStemMute(1, false); setStemMute(2, true); setStemMute(3, true);
        if (separation_available_ && current_stems_.is_valid) {
            emit playStems(3); // DrumsOnly = 3
        }
    });
    connect(bass_only, &QPushButton::clicked, [this]() {
        setStemMute(0, true); setStemMute(1, true); setStemMute(2, false); setStemMute(3, true);
        if (separation_available_ && current_stems_.is_valid) {
            emit playStems(4); // BassOnly = 4
        }
    });
    connect(other_only, &QPushButton::clicked, [this]() {
        setStemMute(0, true); setStemMute(1, true); setStemMute(2, true); setStemMute(3, false);
        if (separation_available_ && current_stems_.is_valid) {
            emit playStems(5); // OtherOnly = 5
        }
    });
    connect(reset_button, &QPushButton::clicked, this, &StemMixerWidget::reset);
    
    // Set initial state
    updateMasterUI();
}

void StemMixerWidget::setupStemControl(StemControl& control, const QString& name, const QColor& color) {
    // Label with colored background
    control.label = new QLabel(name);
    control.label->setStyleSheet(QString(
        "QLabel { "
        "  background-color: %1; "
        "  color: white; "
        "  padding: 4px 8px; "
        "  border-radius: 3px; "
        "  font-weight: bold; "
        "}"
    ).arg(color.name()));
    control.label->setMinimumWidth(60);
    control.label->setAlignment(Qt::AlignCenter);
    
    // Volume slider (0-100)
    control.volume_slider = new QSlider(Qt::Horizontal);
    control.volume_slider->setRange(0, 100);
    control.volume_slider->setValue(100);
    control.volume_slider->setMinimumWidth(100);
    
    // Volume spinbox
    control.volume_spinbox = new QSpinBox();
    control.volume_spinbox->setRange(0, 100);
    control.volume_spinbox->setValue(100);
    control.volume_spinbox->setSuffix("%");
    control.volume_spinbox->setMaximumWidth(60);
    
    // Solo button
    control.solo_button = new QPushButton("S");
    control.solo_button->setCheckable(true);
    control.solo_button->setMaximumWidth(30);
    control.solo_button->setToolTip("Solo this stem");
    control.solo_button->setStyleSheet(
        "QPushButton:checked { background-color: #ffaa00; font-weight: bold; }"
    );
    
    // Mute button  
    control.mute_button = new QPushButton("M");
    control.mute_button->setCheckable(true);
    control.mute_button->setMaximumWidth(30);
    control.mute_button->setToolTip("Mute this stem");
    control.mute_button->setStyleSheet(
        "QPushButton:checked { background-color: #ff4444; color: white; font-weight: bold; }"
    );
    
    // Status label
    control.status_label = new QLabel("N/A");
    control.status_label->setMinimumWidth(50);
    control.status_label->setStyleSheet("font-size: 11px; color: #666;");
    control.status_label->setAlignment(Qt::AlignCenter);
}

void StemMixerWidget::setStemSeparator(StemSeparator* separator) {
    if (stem_separator_) {
        disconnect(stem_separator_, nullptr, this, nullptr);
    }
    
    stem_separator_ = separator;
    
    if (stem_separator_) {
        current_job_id_.clear();
        current_stems_ = SeparatedStems();
        
        connect(stem_separator_, &StemSeparator::separationStarted,
                this, &StemMixerWidget::onSeparationStarted);
        connect(stem_separator_, &StemSeparator::separationProgress,
                this, &StemMixerWidget::onSeparationProgress);
        connect(stem_separator_, &StemSeparator::separationFinished,
                this, &StemMixerWidget::onSeparationFinished);
        connect(stem_separator_, &StemSeparator::separationError,
                this, &StemMixerWidget::onSeparationError);
        
        separation_enabled_ = true;
        enable_checkbox_->setChecked(true);
        
        qLog(Info) << "StemSeparator connected and enabled by default";
    }
    
    updateMasterUI();
}

void StemMixerWidget::setCurrentTrack(const QString& audio_file) {
    current_track_ = audio_file;
    current_job_id_.clear();
    current_stems_ = SeparatedStems();
    
    QFileInfo info(audio_file);
    if (info.exists()) {
        status_label_->setText(QString("Track: %1").arg(info.fileName()));
        separation_available_ = (stem_separator_ && stem_separator_->isInitialized());
        
        if (stem_separator_ && stem_separator_->hasCachedStems(audio_file)) {
            status_label_->setText(QString("Track: %1 (cached stems available)").arg(info.fileName()));
            current_stems_ = stem_separator_->getCachedStems(audio_file);
            separation_available_ = current_stems_.is_valid;
        }
        
        qLog(Info) << "Set current track:" << info.fileName() << "separation_available_:" << separation_available_;
    } else {
        status_label_->setText("No track loaded");
        separation_available_ = false;
        qLog(Warning) << "Invalid track file:" << audio_file;
    }
    
    updateMasterUI();
}

void StemMixerWidget::setSeparationAvailable(bool available) {
    separation_available_ = available;
    updateMasterUI();
}

void StemMixerWidget::setStemVolume(int stem_index, int volume) {
    if (stem_index < 0 || stem_index >= STEM_COUNT) return;
    
    StemControl& control = stem_controls_[stem_index];
    control.volume = qBound(0, volume, 100);
    control.volume_slider->setValue(control.volume);
    control.volume_spinbox->setValue(control.volume);
    
    updateStemUI(stem_index);
    emit stemVolumeChanged(stem_index, control.volume);
}

int StemMixerWidget::getStemVolume(int stem_index) const {
    if (stem_index < 0 || stem_index >= STEM_COUNT) return 100;
    return stem_controls_[stem_index].volume;
}

void StemMixerWidget::setStemSolo(int stem_index, bool solo) {
    if (stem_index < 0 || stem_index >= STEM_COUNT) return;
    
    StemControl& control = stem_controls_[stem_index];
    control.is_solo = solo;
    control.solo_button->setChecked(solo);
    
    if (solo) {
        for (int i = 0; i < STEM_COUNT; ++i) {
            if (i != stem_index && stem_controls_[i].is_solo) {
                setStemSolo(i, false);
            }
        }
        setStemMute(stem_index, false);
    }
    
    updateStemUI(stem_index);
    emit stemSoloChanged(stem_index, solo);
}

bool StemMixerWidget::isStemSolo(int stem_index) const {
    if (stem_index < 0 || stem_index >= STEM_COUNT) return false;
    return stem_controls_[stem_index].is_solo;
}

void StemMixerWidget::setStemMute(int stem_index, bool mute) {
    if (stem_index < 0 || stem_index >= STEM_COUNT) return;
    
    StemControl& control = stem_controls_[stem_index];
    control.is_mute = mute;
    control.mute_button->setChecked(mute);
    
    if (mute && control.is_solo) {
        setStemSolo(stem_index, false);
    }
    
    updateStemUI(stem_index);
    emit stemMuteChanged(stem_index, mute);
}

bool StemMixerWidget::isStemMute(int stem_index) const {
    if (stem_index < 0 || stem_index >= STEM_COUNT) return false;
    return stem_controls_[stem_index].is_mute;
}

void StemMixerWidget::reset() {
    qLog(Info) << "Resetting stem mixer to defaults";
    
    for (int i = 0; i < STEM_COUNT; ++i) {
        setStemVolume(i, 100);
        setStemSolo(i, false);
        setStemMute(i, false);
    }
    
    status_label_->setText("Reset to defaults");
}

void StemMixerWidget::onSeparationStarted(const QString& job_id) {
    current_job_id_ = job_id;
    progress_bar_->setVisible(true);
    progress_bar_->setValue(0);
    status_label_->setText("🤖 AI is separating stems...");
    separate_button_->setEnabled(false);
    
    qLog(Info) << "Stem separation started:" << job_id;
}

void StemMixerWidget::onSeparationProgress(const QString& job_id, int progress) {
    if (job_id != current_job_id_) return;
    
    progress_bar_->setValue(progress);
    status_label_->setText(QString("🤖 AI processing... %1%").arg(progress));
}

void StemMixerWidget::onSeparationFinished(const QString& job_id, const SeparatedStems& stems) {
    if (job_id != current_job_id_) return;
    
    current_stems_ = stems;
    separation_available_ = stems.is_valid;
    current_job_id_.clear();
    
    progress_bar_->setVisible(false);
    separate_button_->setEnabled(true);
    cancel_button_->setVisible(false);
    cancel_button_->setEnabled(false);
    
    if (stems.is_valid) {
        // Mark that we are in offline (file) implementation mode now
        using_original_implementation_ = false;
        status_label_->setText("✅ Stems separated successfully! Use controls below to mix stems.");
        stems_group_->setEnabled(true);
        presets_group_->setEnabled(true);
        
        // Update stem controls to show they're ready
        for (int i = 0; i < STEM_COUNT; ++i) {
            stem_controls_[i].status_label->setText("Ready");
        }
        
        // Log available stem files for debugging
        qLog(Info) << "Available stems:";
        qLog(Info) << "  Vocals:" << stems.vocals_file;
        qLog(Info) << "  Drums:" << stems.drums_file;
        qLog(Info) << "  Bass:" << stems.bass_file;
        qLog(Info) << "  Other:" << stems.other_file;
        
        // Emit signal that stems are ready for main window to handle
        emit stemsReady(stems);
        
    } else {
        status_label_->setText("❌ Stem separation failed");
        stems_group_->setEnabled(false);
        presets_group_->setEnabled(false);
    }
    
    updateMasterUI();
    updateModeIndicator();
    qLog(Info) << "Stem separation finished:" << job_id << "success:" << stems.is_valid;
}

void StemMixerWidget::onSeparationError(const QString& job_id, const QString& error) {
    if (job_id != current_job_id_) return;
    
    current_job_id_.clear();
    progress_bar_->setVisible(false);
    separate_button_->setEnabled(true);
    status_label_->setText(QString("❌ Error: %1").arg(error));
    
    updateMasterUI();
    qLog(Error) << "Stem separation error:" << job_id << error;
}

void StemMixerWidget::onEnableCheckboxChanged(bool checked) {
    separation_enabled_ = checked;
    updateMasterUI();
    emit separationEnabledChanged(checked);
    
    qLog(Info) << "Stem separation enabled:" << checked;
}

void StemMixerWidget::onSeparateButtonClicked() {
    if (!stem_separator_ || current_track_.isEmpty()) {
        qLog(Warning) << "Cannot start separation - stem_separator_:" << (stem_separator_ != nullptr) 
                      << "current_track_:" << current_track_;
        return;
    }
    
    if (!current_job_id_.isEmpty()) {
        qLog(Warning) << "Clearing stuck job ID:" << current_job_id_;
        current_job_id_.clear();
    }
    
    qLog(Info) << "User requested stem separation for:" << current_track_;
    
    // Stop main player before starting stem separation
    emit requestStopMainPlayer();
    
    emit separationRequested(current_track_);
    
    QString job_id = stem_separator_->startSeparation(current_track_);
    if (job_id.isEmpty()) {
        status_label_->setText("❌ Failed to start separation");
        qLog(Error) << "Failed to start stem separation for:" << current_track_;
    } else {
        current_job_id_ = job_id;
        qLog(Info) << "Started stem separation job:" << job_id;
        updateMasterUI();
    }
}

void StemMixerWidget::onLiveButtonClicked() {
    if (current_track_.isEmpty()) {
        status_label_->setText("⚠️ No track loaded");
        qCritical() << "Cannot start live separation: no track loaded";
        return;
    }
    
    if (!QFile::exists(current_track_)) {
        status_label_->setText("⚠️ Track file not found");
        qCritical() << "Track file not found:" << current_track_;
        return;
    }
    
    qDebug() << "User requested ORIGINAL live stem separation for:" << current_track_;
    
    // Stop main player before starting live separation  
    emit requestStopMainPlayer();
    
    // Use original stem implementation
    startOriginalStemSeparation(current_track_);
    
    updateMasterUI();
}

void StemMixerWidget::onPlayPauseButtonClicked() {
    // For offline separation mode, check if stems are available
    if (!using_original_implementation_) {
        if (!current_stems_.is_valid) {
            status_label_->setText("⚠️ No separated stems available");
            qCritical() << "Cannot play stems: No separated stems available";
            return;
        }
        
        // Use standard Clementine player for offline stems
        if (stem_playback_active_) {
            // Currently playing - pause stem engine (not the main player)
            qDebug() << "User requested pause of offline stem playback";
            emit pauseStems(); // Routed to MainWindow::OnPauseStems -> stem_engine_->pause()
            play_pause_button_->setText("▶️ Resume Stems");
            status_label_->setText("⏸️ Stem playback paused");
            stem_playback_active_ = false;
        } else {
            // Currently stopped/paused - play using main player
            qDebug() << "User requested play of stem playback";
            
            // Play the currently selected stem(s)
            emit playStems(0); // All stems
            
            play_pause_button_->setText("⏸️ Pause Stems");
            status_label_->setText("▶️ Playing separated stems");
            stem_playback_active_ = true;
        }
        updateMasterUI();
        return;
    }
    
    // For live separation mode with original implementation
    if (!original_stem_process_) {
        status_label_->setText("⚠️ Original stem player not ready");
        qCritical() << "Cannot control stems: Original player not ready";
        return;
    }
    
    if (stem_playback_active_) {
        // Currently playing - pause
        qDebug() << "User requested pause of original stem playback";
        QJsonObject command;
        command["action"] = "pause";
        sendOriginalStemCommand(command);
        
        play_pause_button_->setText("▶️ Resume Stems");
        status_label_->setText("⏸️ Stem playback paused");
    } else {
        // Currently stopped/paused - play
        qDebug() << "User requested play of original stem playback";
        
        // Stop main player before starting stem playback
        emit requestStopMainPlayer();
        
        QJsonObject command;
        command["action"] = "play";
        sendOriginalStemCommand(command);
        
        play_pause_button_->setText("⏸️ Pause Stems");
        status_label_->setText("▶️ Playing separated stems");
    }
    updateMasterUI();
}

void StemMixerWidget::onStopButtonClicked() {
    // Offline (GstStemEngine) playback path
    if (!using_original_implementation_) {
        if (stem_playback_active_) {
            qDebug() << "User requested stop of offline stem playback";
            emit stopStems();            // Routed to MainWindow::OnStopStems
            stem_playback_active_ = false;
            play_pause_button_->setText("▶️ Play Stems");
            status_label_->setText("⏹️ Stem playback stopped");
        } else {
            qDebug() << "Stop requested but offline stem playback already stopped";
            status_label_->setText("ℹ️ Stem playback already stopped");
        }
        updateMasterUI();
        return;
    }

    // Original (live) implementation path
    if (!original_stem_process_) {
        status_label_->setText("⚠️ Original stem player not ready");
        qCritical() << "Cannot stop: original stem process not ready";
        return;
    }

    qDebug() << "User requested stop of original stem playback";

    QJsonObject command;
    command["action"] = "stop";
    sendOriginalStemCommand(command);

    play_pause_button_->setText("▶️ Play Stems");
    status_label_->setText("⏹️ Stem playback stopped");
    stem_playback_active_ = false;

    updateMasterUI();
}

void StemMixerWidget::onVolumeSliderChanged(int value) {
    int stem_index = getStemIndex(sender());
    if (stem_index >= 0) {
        setStemVolume(stem_index, value);
        
    // Update original stem controller if active
        if (using_original_implementation_ && original_stem_process_) {
            float volume = value / 100.0f;
            QJsonObject command;
            command["action"] = "stem_control";
            command["stem"] = stem_index;
            command["type"] = "volume";
            command["value"] = volume;
            sendOriginalStemCommand(command);
        }
        // Update Python controller if ready (fallback)
        else if (python_controller_ && python_controller_->isReady()) {
            float volume = value / 100.0f;
            python_controller_->setStemVolume(stem_index, volume);
        }
    // Persist volume
    QSettings s("Clementine","StemMixer"); s.beginGroup("controls");
    s.setValue(QString::number(stem_index)+"/volume", value); s.endGroup();
    }
}

void StemMixerWidget::onSoloButtonClicked() {
    int stem_index = getStemIndex(sender());
    if (stem_index >= 0) {
        bool solo = stem_controls_[stem_index].solo_button->isChecked();
        setStemSolo(stem_index, solo);
        
    // Update original stem controller if active
        if (using_original_implementation_ && original_stem_process_) {
            QJsonObject command;
            command["action"] = "stem_control";
            command["stem"] = stem_index;
            command["type"] = "solo";
            command["value"] = solo;
            sendOriginalStemCommand(command);
        }
        // Update Python controller if ready (fallback)
        else if (python_controller_ && python_controller_->isReady()) {
            python_controller_->setStemSolo(stem_index, solo);
        }
    // Persist solo
    QSettings s("Clementine","StemMixer"); s.beginGroup("controls");
    s.setValue(QString::number(stem_index)+"/solo", solo); s.endGroup();
    }
}

void StemMixerWidget::onMuteButtonClicked() {
    int stem_index = getStemIndex(sender());
    if (stem_index >= 0) {
        bool mute = stem_controls_[stem_index].mute_button->isChecked();
        setStemMute(stem_index, mute);
        
    // Update original stem controller if active
        if (using_original_implementation_ && original_stem_process_) {
            QJsonObject command;
            command["action"] = "stem_control";
            command["stem"] = stem_index;
            command["type"] = "mute";
            command["value"] = mute;
            sendOriginalStemCommand(command);
        }
        // Update Python controller if ready (fallback)
        else if (python_controller_ && python_controller_->isReady()) {
            python_controller_->setStemMute(stem_index, mute);
        }
    // Persist mute
    QSettings s("Clementine","StemMixer"); s.beginGroup("controls");
    s.setValue(QString::number(stem_index)+"/mute", mute); s.endGroup();
    }
}

void StemMixerWidget::onStemEngineError(const QString& error_message) {
    status_label_->setText(QString("❌ Engine: %1").arg(error_message.left(60)));
    qLog(Error) << "StemMixerWidget received engine error:" << error_message;
    updateMasterUI();
}

void StemMixerWidget::onStemEngineStateChanged(int state) {
    switch(state) {
        case 0: status_label_->setText("⏹️ Stem playback stopped"); break;
        case 1: status_label_->setText("▶️ Stem playback started"); break;
        case 2: status_label_->setText("⏸️ Stem playback paused"); break;
        default: break;
    }
    updateMasterUI();
}

void StemMixerWidget::updateUI() {
    updateMasterUI();
}

void StemMixerWidget::updateMasterUI() {
    bool has_separator = (stem_separator_ && stem_separator_->isInitialized());
    bool has_track = !current_track_.isEmpty();
    bool is_busy = !current_job_id_.isEmpty();
    bool stems_available = hasSeparatedStems();
    bool playback_active = stem_playback_active_;

    // Throttled logging only when state changes
    if (last_ui_state_.has_separator != has_separator ||
        last_ui_state_.has_track != has_track ||
        last_ui_state_.is_busy != is_busy ||
        last_ui_state_.separation_available != separation_available_ ||
        last_ui_state_.separation_enabled != separation_enabled_ ||
        last_ui_state_.stems_available != stems_available ||
        last_ui_state_.playback_active != playback_active) {
        qLog(Debug) << "StemMixerWidget::updateMasterUI() changed" << "sep" << has_separator
                    << "track" << has_track << "busy" << is_busy
                    << "avail" << separation_available_ << "enabled" << separation_enabled_
                    << "stems" << stems_available << "play" << playback_active;
        last_ui_state_.has_separator = has_separator;
        last_ui_state_.has_track = has_track;
        last_ui_state_.is_busy = is_busy;
        last_ui_state_.separation_available = separation_available_;
        last_ui_state_.separation_enabled = separation_enabled_;
        last_ui_state_.stems_available = stems_available;
        last_ui_state_.playback_active = playback_active;
    }

    enable_checkbox_->setEnabled(has_separator);
    separate_button_->setEnabled(has_separator && has_track && !is_busy && separation_enabled_);
    // live_button_ hidden: skip enabling logic while feature deferred

    bool can_play_offline = stems_available && !is_busy && !using_original_implementation_;
    bool can_play_live = using_original_implementation_ && original_live_ready_ && !is_busy;
    play_pause_button_->setEnabled((can_play_offline || can_play_live));
    stop_button_->setEnabled((can_play_offline || can_play_live) && playback_active);

    cancel_button_->setVisible(is_busy);
    cancel_button_->setEnabled(is_busy);

    stems_group_->setEnabled(separation_enabled_ && separation_available_);
    presets_group_->setEnabled(separation_enabled_ && separation_available_);
    updateModeIndicator();
}

void StemMixerWidget::updateModeIndicator() {
    if (!mode_label_) return;
    QString mode = using_original_implementation_ ? "LIVE" : (current_stems_.is_valid ? "OFFLINE" : "-");
    mode_label_->setText("Mode: " + mode);
}

void StemMixerWidget::updateStemUI(int stem_index) {
    if (stem_index < 0 || stem_index >= STEM_COUNT) return;
    
    StemControl& control = stem_controls_[stem_index];
    
    control.status_label->setText(
        control.is_solo ? "SOLO" :
        control.is_mute ? "MUTE" : 
        separation_available_ ? "Ready" : "N/A"
    );
}

void StemMixerWidget::onCancelButtonClicked() {
    qLog(Info) << "Cancel button clicked, current job ID:" << current_job_id_;
    
    if (!stem_separator_) {
        qLog(Warning) << "Cannot cancel: no stem separator";
        return;
    }
    
    if (current_job_id_.isEmpty()) {
        qLog(Warning) << "Cannot cancel: no active job";
        return;
    }
    
    stem_separator_->cancelSeparation(current_job_id_);
    qLog(Info) << "Cancellation requested for job:" << current_job_id_;
    
    // Update UI immediately
    separate_button_->setEnabled(true);
    cancel_button_->setVisible(false);
    cancel_button_->setEnabled(false);
    progress_bar_->setValue(0);
    status_label_->setText("Separation cancelled");
    
    current_job_id_.clear();
}

int StemMixerWidget::getStemIndex(QObject* sender) {
    for (int i = 0; i < STEM_COUNT; ++i) {
        StemControl& control = stem_controls_[i];
        if (sender == control.volume_slider ||
            sender == control.volume_spinbox ||
            sender == control.solo_button ||
            sender == control.mute_button) {
            return i;
        }
    }
    return -1;
}

// Original stem implementation methods
void StemMixerWidget::startOriginalStemSeparation(const QString& audio_file) {
    qLog(Info) << "Starting original stem separation for:" << audio_file;
    
    if (original_stem_process_) {
        qLog(Warning) << "Original stem process already running, stopping it first";
        stopOriginalStemSeparation();
    }
    
    // Create process
    original_stem_process_ = new QProcess(this);
    
    // Connect signals
    connect(original_stem_process_, &QProcess::readyReadStandardOutput,
            this, &StemMixerWidget::onOriginalStemOutput);
    connect(original_stem_process_, &QProcess::readyReadStandardError,
            this, &StemMixerWidget::onOriginalStemError);
    connect(original_stem_process_, QOverload<int>::of(&QProcess::finished),
            this, &StemMixerWidget::onOriginalStemFinished);
    
    // Set up output directory (persist so we can discover pipes on READY)
    QString output_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/stems";
    QFileInfo info(output_dir);
    if (!info.exists()) {
        QDir().mkpath(output_dir);
    }
    original_output_dir_ = output_dir;
    
    // Find the original stem script - try simplified version first
    QString script_path = QCoreApplication::applicationDirPath() + "/../clementine_simple_stem.py";
    if (!QFileInfo(script_path).exists()) {
        // Try original version
        script_path = QCoreApplication::applicationDirPath() + "/../clementine_live_stem_original.py";
        if (!QFileInfo(script_path).exists()) {
            // Try build directory
            script_path = QCoreApplication::applicationDirPath() + "/clementine_simple_stem.py";
        }
    }
    
    // Prepare arguments
    QStringList args;
    args << script_path;
    args << "--audio" << audio_file;
    args << "--output" << output_dir;
    args << "--model" << "htdemucs_ft";
    
    qLog(Info) << "Starting Python process with args:" << args;
    
    // Try to use the configured Python environment if available
    QString python_cmd = "python3";
    QString env_python = QCoreApplication::applicationDirPath() + "/../stem_separation_env/bin/python";
    if (QFileInfo(env_python).exists()) {
        python_cmd = env_python;
        qLog(Info) << "Using configured Python environment:" << python_cmd;
    } else {
        qLog(Info) << "Using system Python:" << python_cmd;
    }
    
    // Start process
    original_stem_process_->start(python_cmd, args);
    
    if (!original_stem_process_->waitForStarted(5000)) {
        qLog(Error) << "Failed to start original stem process";
        delete original_stem_process_;
        original_stem_process_ = nullptr;
        emit originalStemError("Failed to start stem separation process");
        return;
    }
    
    // Update UI
    status_label_->setText("Starting original stem separation...");
    separate_button_->setEnabled(false);
    cancel_button_->setVisible(true);
    cancel_button_->setEnabled(true);
    
    stem_playback_active_ = false;
    using_original_implementation_ = true;
    original_live_ready_ = false; // reset readiness
    updateModeIndicator();
}

void StemMixerWidget::stopOriginalStemSeparation() {
    if (!original_stem_process_) {
        return;
    }
    
    qLog(Info) << "Stopping original stem separation";
    
    original_stem_process_->terminate();
    if (!original_stem_process_->waitForFinished(3000)) {
        qLog(Warning) << "Force killing original stem process";
        original_stem_process_->kill();
    }
    
    original_stem_process_->deleteLater();
    original_stem_process_ = nullptr;
    
    stem_playback_active_ = false;
    using_original_implementation_ = false;
    updateMasterUI();
}

void StemMixerWidget::sendOriginalStemCommand(const QJsonObject& command) {
    if (!original_stem_process_ || original_stem_process_->state() != QProcess::Running) {
        qLog(Warning) << "No active original stem process to send command to";
        return;
    }
    
    QJsonDocument doc(command);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    
    qLog(Debug) << "Sending command to original stem process:" << data;
    original_stem_process_->write(data);
}

void StemMixerWidget::onOriginalStemOutput() {
    if (!original_stem_process_) return;
    
    QByteArray data = original_stem_process_->readAllStandardOutput();
    QString output = QString::fromUtf8(data).trimmed();
    
    qLog(Debug) << "Original stem output:" << output;
    
    // Handle line-by-line output
    QStringList lines = output.split('\n');
    for (const QString& line : lines) {
        if (line.isEmpty()) continue;
        
        if (line == "READY") {
            qLog(Info) << "Original stem separation ready (LIVE pipes primed)";
            status_label_->setText("Stem separation ready (LIVE)");
            original_live_ready_ = true;
            // Enable play button immediately
            play_pause_button_->setEnabled(true);
            // Attempt to discover pipe files created by the simple live script (vocals.pipe, drums.pipe, bass.pipe, other.pipe)
            // and synthesize a SeparatedStems structure so the GstStemEngine can attach in live mode.
            if (!original_output_dir_.isEmpty()) {
                QStringList expected_names = {"vocals.pipe", "drums.pipe", "bass.pipe", "other.pipe"};
                QDir odir(original_output_dir_);
                // Map to SeparatedStems fields (order in engine expects: vocals, drums, bass, other when we assign)
                QString vocals_path = odir.absoluteFilePath("vocals.pipe");
                QString drums_path = odir.absoluteFilePath("drums.pipe");
                QString bass_path = odir.absoluteFilePath("bass.pipe");
                QString other_path = odir.absoluteFilePath("other.pipe");
                bool all_exist = QFileInfo::exists(vocals_path) && QFileInfo::exists(drums_path) && QFileInfo::exists(bass_path) && QFileInfo::exists(other_path);
                if (all_exist) {
                    SeparatedStems live_stems;
                    live_stems.vocals_file = vocals_path;  // engine checks for .pipe to switch to live mode
                    live_stems.drums_file = drums_path;
                    live_stems.bass_file = bass_path;
                    live_stems.other_file = other_path;
                    live_stems.is_valid = true;
                    live_stems.timestamp = QDateTime::currentSecsSinceEpoch();
                    current_stems_ = live_stems;
                    separation_available_ = true; // allow UI controls
                    qLog(Info) << "Discovered live pipe stems:";
                    qLog(Info) << "  Vocals:" << live_stems.vocals_file;
                    qLog(Info) << "  Drums:" << live_stems.drums_file;
                    qLog(Info) << "  Bass:" << live_stems.bass_file;
                    qLog(Info) << "  Other:" << live_stems.other_file;
                    // Reuse existing signal so MainWindow loads them via GstStemEngine
                    emit stemsReady(live_stems);
                } else {
                    qLog(Warning) << "LIVE READY received but some pipe files missing in" << original_output_dir_ << "-- retrying in 500ms";
                    QTimer::singleShot(500, this, [this]() {
                        if (current_stems_.is_valid) return; // already loaded via earlier attempt
                        if (original_output_dir_.isEmpty()) return;
                        QDir odir(original_output_dir_);
                        QString vocals_path = odir.absoluteFilePath("vocals.pipe");
                        QString drums_path = odir.absoluteFilePath("drums.pipe");
                        QString bass_path = odir.absoluteFilePath("bass.pipe");
                        QString other_path = odir.absoluteFilePath("other.pipe");
                        bool all_exist = QFileInfo::exists(vocals_path) && QFileInfo::exists(drums_path) && QFileInfo::exists(bass_path) && QFileInfo::exists(other_path);
                        if (!all_exist) {
                            qLog(Warning) << "Retry: live pipes still not all present";
                            return;
                        }
                        SeparatedStems live_stems;
                        live_stems.vocals_file = vocals_path;
                        live_stems.drums_file = drums_path;
                        live_stems.bass_file = bass_path;
                        live_stems.other_file = other_path;
                        live_stems.is_valid = true;
                        live_stems.timestamp = QDateTime::currentSecsSinceEpoch();
                        current_stems_ = live_stems;
                        separation_available_ = true;
                        qLog(Info) << "Discovered live pipe stems (retry):";
                        qLog(Info) << "  Vocals:" << live_stems.vocals_file;
                        qLog(Info) << "  Drums:" << live_stems.drums_file;
                        qLog(Info) << "  Bass:" << live_stems.bass_file;
                        qLog(Info) << "  Other:" << live_stems.other_file;
                        emit stemsReady(live_stems);
                        updateMasterUI();
                    });
                }
            }
            updateMasterUI();
            emit originalStemReady();
            continue;
        }
        
        // Try to parse JSON status
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject status = doc.object();
            
            if (status.contains("error")) {
                QString error_msg = status["error"].toString();
                qLog(Error) << "Original stem error:" << error_msg;
                emit originalStemError(error_msg);
            } else {
                // Update status from JSON
                bool is_playing = status["is_playing"].toBool();
                double position = status["position"].toDouble();
                double duration = status["duration"].toDouble();
                
                stem_playback_active_ = is_playing;
                
                if (duration > 0) {
                    progress_bar_->setValue(static_cast<int>((position / duration) * 100));
                }
                
                updateMasterUI();
            }
        }
    }
}

void StemMixerWidget::onOriginalStemError() {
    if (!original_stem_process_) return;
    
    QByteArray data = original_stem_process_->readAllStandardError();
    QString error_output = QString::fromUtf8(data).trimmed();
    
    if (!error_output.isEmpty()) {
        qLog(Error) << "Original stem process error:" << error_output;
        emit originalStemError(error_output);
    }
}

void StemMixerWidget::onOriginalStemFinished(int exitCode) {
    qLog(Info) << "Original stem process finished with exit code:" << exitCode;
    
    if (original_stem_process_) {
        original_stem_process_->deleteLater();
        original_stem_process_ = nullptr;
    }
    
    stem_playback_active_ = false;
    using_original_implementation_ = false;
    
    if (exitCode != 0) {
        emit originalStemError(QString("Process exited with code %1").arg(exitCode));
    }
    
    updateMasterUI();
}
