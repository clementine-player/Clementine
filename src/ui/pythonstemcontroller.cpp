#include "pythonstemcontroller.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QJsonArray>
#include <QDebug>

PythonStemController::PythonStemController(QObject* parent)
    : QObject(parent)
    , python_process_(nullptr)
    , status_timer_(new QTimer(this))
    , is_ready_(false)
    , playback_state_(STOPPED)
    , position_(0.0f)
    , duration_(0.0f)
    , progress_(0.0f)
{
    // Initialize stem state
    for (int i = 0; i < 4; ++i) {
        stem_volumes_[i] = 1.0f;
        stem_mutes_[i] = false;
        stem_solos_[i] = false;
    }
    
    // Setup status timer
    status_timer_->setInterval(200); // Update every 200ms
    connect(status_timer_, &QTimer::timeout, this, &PythonStemController::onStatusTimer);
    
    // Find Python script path
    python_script_path_ = QCoreApplication::applicationDirPath() + "/../clementine_stem_player.py";
    if (!QFile::exists(python_script_path_)) {
        // Try alternative locations
        python_script_path_ = QDir::currentPath() + "/clementine_stem_player.py";
    }
    
    qDebug() << "PythonStemController: Script path:" << python_script_path_;
}

PythonStemController::~PythonStemController()
{
    stopPlayer();
}

bool PythonStemController::startPlayer(const QString& audio_file, const QString& output_dir)
{
    if (python_process_ && python_process_->state() != QProcess::NotRunning) {
        qWarning() << "PythonStemController: Player already running";
        return false;
    }
    
    if (!QFile::exists(python_script_path_)) {
        qCritical() << "PythonStemController: Script not found:" << python_script_path_;
        emit playerError("Python script not found: " + python_script_path_);
        return false;
    }
    
    if (!QFile::exists(audio_file)) {
        qCritical() << "PythonStemController: Audio file not found:" << audio_file;
        emit playerError("Audio file not found: " + audio_file);
        return false;
    }
    
    // Create output directory
    QDir().mkpath(output_dir);
    
    // Store parameters
    current_audio_file_ = audio_file;
    current_output_dir_ = output_dir;
    
    // Create and setup process
    python_process_ = new QProcess(this);
    
    connect(python_process_, &QProcess::readyReadStandardOutput,
            this, &PythonStemController::onProcessReadyRead);
    connect(python_process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonStemController::onProcessFinished);
    connect(python_process_, &QProcess::errorOccurred,
            this, &PythonStemController::onProcessError);
    
    // Setup arguments
    QStringList arguments;
    arguments << python_script_path_;
    arguments << "--audio" << audio_file;
    arguments << "--output" << output_dir;
    arguments << "--model" << "htdemucs_ft";
    
    qDebug() << "PythonStemController: Starting process with args:" << arguments;
    
    // Start the process
    python_process_->start("python3", arguments);
    
    if (!python_process_->waitForStarted(5000)) {
        qCritical() << "PythonStemController: Failed to start process:" << python_process_->errorString();
        emit playerError("Failed to start Python process: " + python_process_->errorString());
        return false;
    }
    
    qDebug() << "PythonStemController: Process started successfully";
    return true;
}

void PythonStemController::stopPlayer()
{
    status_timer_->stop();
    
    if (python_process_) {
        if (python_process_->state() != QProcess::NotRunning) {
            python_process_->kill();
            python_process_->waitForFinished(3000);
        }
        python_process_->deleteLater();
        python_process_ = nullptr;
    }
    
    is_ready_ = false;
    playback_state_ = STOPPED;
    position_ = 0.0f;
    progress_ = 0.0f;
    
    emit playbackStateChanged(playback_state_);
}

bool PythonStemController::play()
{
    QJsonObject command;
    command["action"] = "play";
    return sendCommand(command);
}

bool PythonStemController::pause()
{
    QJsonObject command;
    command["action"] = "pause";
    return sendCommand(command);
}

bool PythonStemController::resume()
{
    QJsonObject command;
    command["action"] = "resume";
    return sendCommand(command);
}

bool PythonStemController::stop()
{
    QJsonObject command;
    command["action"] = "stop";
    return sendCommand(command);
}

bool PythonStemController::setStemVolume(int stem_index, float volume)
{
    if (stem_index < 0 || stem_index >= 4) return false;
    
    QJsonObject command;
    command["action"] = "set_volume";
    command["stem"] = stem_index;
    command["volume"] = static_cast<double>(volume);
    
    if (sendCommand(command)) {
        stem_volumes_[stem_index] = volume;
        emit stemVolumeChanged(stem_index, volume);
        return true;
    }
    return false;
}

bool PythonStemController::setStemMute(int stem_index, bool mute)
{
    if (stem_index < 0 || stem_index >= 4) return false;
    
    QJsonObject command;
    command["action"] = "set_mute";
    command["stem"] = stem_index;
    command["mute"] = mute;
    
    if (sendCommand(command)) {
        stem_mutes_[stem_index] = mute;
        emit stemMuteChanged(stem_index, mute);
        return true;
    }
    return false;
}

bool PythonStemController::setStemSolo(int stem_index, bool solo)
{
    if (stem_index < 0 || stem_index >= 4) return false;
    
    QJsonObject command;
    command["action"] = "set_solo";
    command["stem"] = stem_index;
    command["solo"] = solo;
    
    if (sendCommand(command)) {
        stem_solos_[stem_index] = solo;
        emit stemSoloChanged(stem_index, solo);
        return true;
    }
    return false;
}

float PythonStemController::getStemVolume(int stem_index) const
{
    if (stem_index >= 0 && stem_index < 4) {
        return stem_volumes_[stem_index];
    }
    return 1.0f;
}

bool PythonStemController::isStemMute(int stem_index) const
{
    if (stem_index >= 0 && stem_index < 4) {
        return stem_mutes_[stem_index];
    }
    return false;
}

bool PythonStemController::isStemSolo(int stem_index) const
{
    if (stem_index >= 0 && stem_index < 4) {
        return stem_solos_[stem_index];
    }
    return false;
}

void PythonStemController::requestStatus()
{
    QJsonObject command;
    command["action"] = "status";
    sendCommand(command);
}

void PythonStemController::onProcessReadyRead()
{
    if (!python_process_) return;
    
    QByteArray data = python_process_->readAllStandardOutput();
    QStringList lines = QString::fromUtf8(data).split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;
        
        qDebug() << "PythonStemController: Received:" << trimmed;
        
        if (trimmed == "READY") {
            is_ready_ = true;
            status_timer_->start();
            emit playerReady();
        } else if (trimmed.startsWith("ERROR:")) {
            QString error = trimmed.mid(6);
            emit playerError(error);
        } else {
            handleResponse(trimmed);
        }
    }
}

void PythonStemController::onProcessFinished(int exit_code, QProcess::ExitStatus exit_status)
{
    qDebug() << "PythonStemController: Process finished with code:" << exit_code << "status:" << exit_status;
    
    status_timer_->stop();
    is_ready_ = false;
    playback_state_ = STOPPED;
    
    emit playbackStateChanged(playback_state_);
    
    if (exit_status == QProcess::CrashExit) {
        emit playerError("Python process crashed");
    }
}

void PythonStemController::onProcessError(QProcess::ProcessError error)
{
    qCritical() << "PythonStemController: Process error:" << error;
    
    QString error_msg;
    switch (error) {
        case QProcess::FailedToStart:
            error_msg = "Failed to start Python process";
            break;
        case QProcess::Crashed:
            error_msg = "Python process crashed";
            break;
        case QProcess::Timedout:
            error_msg = "Python process timed out";
            break;
        case QProcess::WriteError:
            error_msg = "Write error to Python process";
            break;
        case QProcess::ReadError:
            error_msg = "Read error from Python process";
            break;
        default:
            error_msg = "Unknown process error";
            break;
    }
    
    emit playerError(error_msg);
}

void PythonStemController::onStatusTimer()
{
    if (is_ready_) {
        requestStatus();
    }
}

bool PythonStemController::sendCommand(const QJsonObject& command)
{
    if (!python_process_ || !is_ready_) {
        qWarning() << "PythonStemController: Process not ready for commands";
        return false;
    }
    
    QJsonDocument doc(command);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    
    qDebug() << "PythonStemController: Sending command:" << doc.toJson(QJsonDocument::Compact);
    
    qint64 written = python_process_->write(data);
    if (written != data.size()) {
        qWarning() << "PythonStemController: Failed to write complete command";
        return false;
    }
    
    return true;
}

void PythonStemController::handleResponse(const QString& response)
{
    if (response == "OK" || response == "ERROR") {
        // Simple acknowledge responses
        return;
    }
    
    // Try to parse as JSON status
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8(), &error);
    
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        updateStatusFromJson(doc.object());
    } else {
        qDebug() << "PythonStemController: Non-JSON response:" << response;
    }
}

void PythonStemController::updateStatusFromJson(const QJsonObject& status)
{
    // Update playback state
    QString state_str = status["state"].toString();
    PlaybackState new_state = STOPPED;
    
    if (state_str == "PLAYING") {
        new_state = PLAYING;
    } else if (state_str == "PAUSED") {
        new_state = PAUSED;
    }
    
    if (new_state != playback_state_) {
        playback_state_ = new_state;
        emit playbackStateChanged(playback_state_);
    }
    
    // Update position and progress
    float new_position = static_cast<float>(status["position"].toDouble());
    float new_duration = static_cast<float>(status["duration"].toDouble());
    float new_progress = static_cast<float>(status["progress"].toDouble());
    
    if (qAbs(new_position - position_) > 0.1f) {
        position_ = new_position;
        emit positionChanged(position_);
    }
    
    if (qAbs(new_progress - progress_) > 0.01f) {
        progress_ = new_progress;
        emit progressChanged(progress_);
    }
    
    duration_ = new_duration;
    
    // Update stem states
    QJsonArray volumes = status["stem_volumes"].toArray();
    QJsonArray mutes = status["stem_mutes"].toArray();
    QJsonArray solos = status["stem_solos"].toArray();
    
    for (int i = 0; i < 4 && i < volumes.size(); ++i) {
        float volume = static_cast<float>(volumes[i].toDouble());
        if (qAbs(volume - stem_volumes_[i]) > 0.01f) {
            stem_volumes_[i] = volume;
            emit stemVolumeChanged(i, volume);
        }
    }
    
    for (int i = 0; i < 4 && i < mutes.size(); ++i) {
        bool mute = mutes[i].toBool();
        if (mute != stem_mutes_[i]) {
            stem_mutes_[i] = mute;
            emit stemMuteChanged(i, mute);
        }
    }
    
    for (int i = 0; i < 4 && i < solos.size(); ++i) {
        bool solo = solos[i].toBool();
        if (solo != stem_solos_[i]) {
            stem_solos_[i] = solo;
            emit stemSoloChanged(i, solo);
        }
    }
}
