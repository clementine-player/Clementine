#include "stemseparator.h"
#include "core/logging.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDateTime>
#include <QUuid>
#include <QTimer>
#include <QCoreApplication>
#include <QProcess>

namespace {
    const QString kDefaultModel = "htdemucs";
    const QString kCacheSubdir = "clementine/stem_cache";
    const qint64 kDefaultMaxCacheSize = 1024; // 1GB
    const QString kPythonScript = "clementine_stem_wrapper.sh";
    // Extended timeout (15 minutes) for heavy models & first-run model downloads
    const int kSeparationTimeoutMs = 900000; // 15 * 60 * 1000
}

StemSeparator::StemSeparator(QObject* parent)
    : QObject(parent),
      initialized_(false),
      model_name_(kDefaultModel),
      cache_enabled_(true),
      max_cache_size_mb_(kDefaultMaxCacheSize),
      python_module_(nullptr),
      python_available_(false) {
      
    // Register SeparatedStems as Qt metatype for signal/slot connections
    qRegisterMetaType<SeparatedStems>("SeparatedStems");
    
    // Set up cache directory
    cache_directory_ = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" + kCacheSubdir;
    QDir().mkpath(cache_directory_);
    
    // Set up output directory
    output_directory_ = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/clementine_stems";
    QDir().mkpath(output_directory_);
    
    qLog(Info) << "StemSeparator created with cache dir:" << cache_directory_;
}

StemSeparator::~StemSeparator() {
    cleanup();
}

bool StemSeparator::initialize() {
    if (initialized_) {
        return true;
    }
    
    qLog(Info) << "Initializing AI Stem Separator...";
    
    // Check if Python environment is available
    if (checkPythonEnvironment()) {
        qLog(Info) << "✅ Python environment with Demucs found - AI separation enabled";
        initialized_ = true;
        return true;
    } else {
        qLog(Warning) << "⚠️ Python/Demucs not available - using mock separation mode";
        initialized_ = true; // Still initialize for UI testing
        return true;
    }
}

void StemSeparator::cleanup() {
    // Clean up active jobs
    for (auto job : active_jobs_.values()) {
        job->cancel();
        job->deleteLater();
    }
    active_jobs_.clear();
    initialized_ = false;
}

QString StemSeparator::startSeparation(const QString& audio_file) {
    if (!initialized_) {
        last_error_ = "StemSeparator not initialized";
        qLog(Error) << last_error_;
        return QString();
    }
    
    QFileInfo file_info(audio_file);
    if (!file_info.exists()) {
        last_error_ = QString("Audio file not found: %1").arg(audio_file);
        qLog(Error) << last_error_;
        return QString();
    }
    
    QString job_id = generateJobId();
    qLog(Info) << "Starting stem separation job:" << job_id << "for:" << file_info.fileName();
    
    // Create and start background job
    StemSeparationJob* job = new StemSeparationJob(
        job_id, audio_file, model_name_, output_directory_, this);
    
    connect(job, &StemSeparationJob::progressUpdate, 
            this, &StemSeparator::separationProgress);
    connect(job, &StemSeparationJob::finished,
            this, &StemSeparator::onSeparationFinished);
    connect(job, &StemSeparationJob::error,
            this, &StemSeparator::onSeparationError);
    
    jobs_mutex_.lock();
    active_jobs_[job_id] = job;
    jobs_mutex_.unlock();
    
    job->start();
    emit separationStarted(job_id);
    
    return job_id;
}

QString StemSeparator::startLiveSeparation(const QString& audio_file) {
    if (!initialized_) {
        last_error_ = "StemSeparator not initialized";
        qLog(Error) << last_error_;
        return QString();
    }
    
    QFileInfo file_info(audio_file);
    if (!file_info.exists()) {
        last_error_ = QString("Audio file not found: %1").arg(audio_file);
        qLog(Error) << last_error_;
        return QString();
    }
    
    QString job_id = generateJobId();
    qLog(Info) << "Starting LIVE stem separation job:" << job_id << "for:" << file_info.fileName();
    
    // Create unique output directory for live pipes
    QString live_output_dir = output_directory_ + "/live_" + job_id;
    QDir().mkpath(live_output_dir);
    
    // Use the live Python script instead
    QString live_script_path;
    if (!python_script_path_.isEmpty()) {
        live_script_path = python_script_path_;
        live_script_path.replace("clementine_stem_wrapper.sh", "clementine_live_stem.py");
    } else {
        // In mock mode, search for the live script directly
        QStringList search_paths = {
            QCoreApplication::applicationDirPath(),
            QCoreApplication::applicationDirPath() + "/../",
            ".",
            ".."
        };
        
        for (const QString& path : search_paths) {
            QFileInfo script_info(QDir(path).absoluteFilePath("clementine_live_stem.py"));
            if (script_info.exists()) {
                live_script_path = script_info.absoluteFilePath();
                break;
            }
        }
    }
    
    qLog(Info) << "Original script path:" << python_script_path_;
    qLog(Info) << "Live script path:" << live_script_path;
    
    // Create and start live background job
    LiveStemSeparationJob* live_job = new LiveStemSeparationJob(
        job_id, audio_file, model_name_, live_output_dir, live_script_path, python_executable_, this);
    
    connect(live_job, &LiveStemSeparationJob::progressUpdate, 
            this, &StemSeparator::separationProgress);
    connect(live_job, &LiveStemSeparationJob::liveStemsReady,
            this, &StemSeparator::liveStemsReady);
    connect(live_job, &LiveStemSeparationJob::error,
            this, &StemSeparator::onSeparationError);
    
    jobs_mutex_.lock();
    active_jobs_[job_id] = live_job;  // Store as base class pointer
    jobs_mutex_.unlock();
    
    live_job->start();
    emit separationStarted(job_id);
    
    return job_id;
}

StemSeparator::ProcessingState StemSeparator::getProcessingState(const QString& job_id) const {
    jobs_mutex_.lock();
    
    if (active_jobs_.contains(job_id)) {
        jobs_mutex_.unlock();
        return Processing;
    }
    if (completed_jobs_.contains(job_id)) {
        jobs_mutex_.unlock();
        return Completed;
    }
    jobs_mutex_.unlock();
    return Idle;
}

SeparatedStems StemSeparator::getResults(const QString& job_id) const {
    jobs_mutex_.lock();
    SeparatedStems result = completed_jobs_.value(job_id);
    jobs_mutex_.unlock();
    return result;
}

void StemSeparator::cancelSeparation(const QString& job_id) {
    jobs_mutex_.lock();
    
    if (BaseSeparationJob* job = active_jobs_.value(job_id)) {
        job->cancel();
        qLog(Info) << "Cancelled stem separation job:" << job_id;
    }
    
    jobs_mutex_.unlock();
}

bool StemSeparator::hasCachedStems(const QString& audio_file) const {
    QString cache_key = getCacheKey(audio_file);
    cache_mutex_.lock();
    bool result = cache_.contains(cache_key);
    cache_mutex_.unlock();
    return result;
}

SeparatedStems StemSeparator::getCachedStems(const QString& audio_file) const {
    QString cache_key = getCacheKey(audio_file);
    cache_mutex_.lock();
    SeparatedStems result = cache_.value(cache_key);
    cache_mutex_.unlock();
    return result;
}

void StemSeparator::clearCache() {
    cache_mutex_.lock();
    cache_.clear();
    cache_mutex_.unlock();
    
    // Also clear cache files
    QDir cache_dir(cache_directory_);
    cache_dir.removeRecursively();
    cache_dir.mkpath(cache_directory_);
    
    qLog(Info) << "Stem cache cleared";
}

void StemSeparator::setModelName(const QString& model) {
    model_name_ = model;
    qLog(Info) << "AI model set to:" << model_name_;
}

void StemSeparator::setOutputDirectory(const QString& dir) {
    output_directory_ = dir;
    QDir().mkpath(output_directory_);
    qLog(Info) << "Output directory set to:" << output_directory_;
}

void StemSeparator::setCacheEnabled(bool enabled) {
    cache_enabled_ = enabled;
    qLog(Info) << "Stem cache" << (enabled ? "enabled" : "disabled");
}

QString StemSeparator::generateJobId() const {
    return QUuid::createUuid().toString().remove('{').remove('}');
}

QString StemSeparator::getCacheKey(const QString& audio_file) const {
    QFileInfo file_info(audio_file);
    QString combined = file_info.absoluteFilePath() + "|" + 
                      QString::number(file_info.lastModified().toSecsSinceEpoch()) + "|" +
                      model_name_;
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(combined.toUtf8());
    return hash.result().toHex();
}

void StemSeparator::saveToCache(const QString& cache_key, const SeparatedStems& stems) {
    if (!cache_enabled_) return;
    
    cache_mutex_.lock();
    cache_[cache_key] = stems;
    cache_mutex_.unlock();
    
    qLog(Info) << "Stems cached with key:" << cache_key;
}

void StemSeparator::cleanupFinishedJobs() {
    jobs_mutex_.lock();
    
    // Remove old completed jobs to free memory
    auto it = completed_jobs_.begin();
    while (it != completed_jobs_.end()) {
        // Keep only the last 10 completed jobs
        if (completed_jobs_.size() > 10) {
            it = completed_jobs_.erase(it);
        } else {
            ++it;
        }
    }
    
    jobs_mutex_.unlock();
    qLog(Info) << "Finished jobs cleaned up";
}

void StemSeparator::onProgressUpdate(const QString& job_id, int progress) {
    emit separationProgress(job_id, progress);
}

bool StemSeparator::checkPythonEnvironment() {
    // Check if our Python script exists
    QStringList search_paths = {
        QCoreApplication::applicationDirPath(),
        QCoreApplication::applicationDirPath() + "/../",
        ".",
        ".."
    };
    
    QString script_path;
    for (const QString& path : search_paths) {
        QFileInfo script_info(QDir(path).absoluteFilePath(kPythonScript));
        if (script_info.exists()) {
            script_path = script_info.absoluteFilePath();
            break;
        }
    }
    
    if (script_path.isEmpty()) {
        qLog(Warning) << "Python script not found:" << kPythonScript;
        return false;
    }
    
    // Test if Python environment works
    QProcess test_process;
    
    // Try virtual environment first - look in project directory
    QStringList venv_paths = {
        "/home/stefan/Dokumente/Programmieren lernen/AIMP-Clementine/stem_separation_env/bin/python",
        QDir::currentPath() + "/stem_separation_env/bin/python",
        QDir::currentPath() + "/../stem_separation_env/bin/python",
        QFileInfo(script_path).dir().absolutePath() + "/stem_separation_env/bin/python"
    };
    
    QString python_executable = "python3";
    for (const QString& venv_path : venv_paths) {
        if (QFile::exists(venv_path)) {
            python_executable = venv_path;
            qLog(Info) << "Using virtual environment Python:" << venv_path;
            break;
        }
    }
    
    if (python_executable == "python3") {
        qLog(Info) << "Using system Python3";
    }
    
    test_process.setProgram(python_executable);
    
    test_process.setArguments({"-c", "import demucs; print('OK')"});
    test_process.start();
    test_process.waitForFinished(5000);
    
    if (test_process.exitCode() == 0) {
        python_script_path_ = script_path;
        python_executable_ = python_executable;  // Store the executable path
        python_available_ = true;
        qLog(Info) << "Python script found at:" << script_path;
        qLog(Info) << "Using Python executable:" << python_executable;
        return true;
    } else {
        python_available_ = false;
        qLog(Warning) << "Python/Demucs test failed:" << test_process.readAllStandardError();
        return false;
    }
}

void StemSeparator::onSeparationFinished(const QString& job_id, const SeparatedStems& stems) {
    qLog(Info) << "Stem separation completed for job:" << job_id;
    
    jobs_mutex_.lock();
    
    // Move from active to completed
    if (BaseSeparationJob* job = active_jobs_.take(job_id)) {
        completed_jobs_[job_id] = stems;
        
        // Cache the results (only for regular separation jobs)
        if (cache_enabled_ && stems.is_valid) {
            // Try to cast to StemSeparationJob to get audio file path
            if (StemSeparationJob* reg_job = dynamic_cast<StemSeparationJob*>(job)) {
                QString cache_key = getCacheKey(reg_job->getAudioFile());
                saveToCache(cache_key, stems);
            }
        }
        
        // Schedule job cleanup
        job->deleteLater();
    }
    
    jobs_mutex_.unlock();
    
    emit separationFinished(job_id, stems);
}

void StemSeparator::onSeparationError(const QString& job_id, const QString& error) {
    qLog(Error) << "Stem separation error for job:" << job_id << error;
    
    jobs_mutex_.lock();
    if (BaseSeparationJob* job = active_jobs_.take(job_id)) {
        job->deleteLater();
    }
    jobs_mutex_.unlock();
    
    last_error_ = error;
    emit separationError(job_id, error);
}

// StemSeparationJob implementation
StemSeparationJob::StemSeparationJob(const QString& job_id,
                                     const QString& audio_file,
                                     const QString& model_name,
                                     const QString& output_dir,
                                     QObject* parent)
    : BaseSeparationJob(parent),
      job_id_(job_id),
      audio_file_(audio_file),
      model_name_(model_name),
      output_dir_(output_dir),
      cancelled_(false) {
}

void StemSeparationJob::run() {
    qLog(Info) << "StemSeparationJob started:" << job_id_;
    emit progressUpdate(job_id_, 5);
    
    if (cancelled_) return;
    
    // Check if Python script is available
    StemSeparator* parent_separator = qobject_cast<StemSeparator*>(parent());
    bool use_python = parent_separator && parent_separator->isPythonAvailable();
    
    if (use_python) {
        runPythonSeparation();
    } else {
        runMockSeparation();
    }
}

void StemSeparationJob::runPythonSeparation() {
    qLog(Info) << "Running real AI stem separation for:" << QFileInfo(audio_file_).fileName();
    
    StemSeparator* parent_separator = qobject_cast<StemSeparator*>(parent());
    if (!parent_separator) {
        emit error(job_id_, "Invalid parent separator");
        return;
    }
    
    // Prepare output directory
    QString job_output_dir = output_dir_ + "/" + job_id_;
    QDir().mkpath(job_output_dir);
    
    // Run Python separation
    QProcess python_process;

    // Use bash + wrapper script to avoid argument parsing issues and ensure unbuffered output
    python_process.setProgram("/bin/bash");

    QStringList args;
    args << parent_separator->getPythonScriptPath();            // wrapper script path
    args << "--audio" << audio_file_;
    args << "--output" << job_output_dir;
    args << "--model" << model_name_;
    python_process.setArguments(args);

    qLog(Info) << "Executing:" << python_process.program() << args.join(" ");

    qint64 start_ms = QDateTime::currentMSecsSinceEpoch();
    python_process.start();
    if (!python_process.waitForStarted(10000)) {
        emit error(job_id_, "Failed to start separation process");
        return;
    }

    // Emit an early small progress bump so UI shows activity beyond fixed 5%
    emit progressUpdate(job_id_, 6);
    qLog(Info) << "StemSeparationJob: entered poll-based stdout parsing loop";

    // Real-time progress parsing from Python stdout (lines starting with PROGRESS:<n>)
    QByteArray stdout_buffer;          // rolling buffer (unprocessed part)
    QByteArray stderr_buffer;          // rolling buffer (unprocessed part)
    QByteArray full_stdout_accum;      // full accumulated stdout for diagnostics
    QByteArray full_stderr_accum;      // full accumulated stderr for diagnostics
    int last_reported = 5; // already emitted 5% before entering here
    qint64 last_progress_ms = start_ms;
    bool saw_separation_complete_marker = false;

    auto process_stdout_lines = [&](bool final_flush){
        int newline_index;
        while ((newline_index = stdout_buffer.indexOf('\n')) != -1) {
            QByteArray line_bytes = stdout_buffer.left(newline_index);
            stdout_buffer.remove(0, newline_index + 1);
            QString line = QString::fromUtf8(line_bytes).trimmed();
            if (line.isEmpty()) continue;
            // Keep all lines for post-mortem if needed
            if (line.startsWith("PROGRESS:")) {
                bool ok = false;
                int value = line.mid(9).toInt(&ok);
                if (ok && value >= 0 && value <= 100) {
                    if (value > last_reported) {
                        emit progressUpdate(job_id_, value);
                        last_reported = value;
                        last_progress_ms = QDateTime::currentMSecsSinceEpoch();
                    }
                }
            } else if (line == "SEPARATION_COMPLETE") {
                saw_separation_complete_marker = true;
                if (last_reported < 95) {
                    emit progressUpdate(job_id_, 95);
                    last_reported = 95;
                }
            } else if (line.startsWith("ERROR:")) {
                qLog(Error) << "Python reported error line:" << line;
            } else if (final_flush) {
                // On final flush, log any remaining non-progress lines at debug
                qLog(Debug) << "Python stdout:" << line;
            }
        }
    };

    // Poll loop with explicit timeout accounting to avoid premature waitForFinished issues
    while (!cancelled_) {
        // Refresh buffers
        if (python_process.waitForReadyRead(500)) {
            QByteArray out_now = python_process.readAllStandardOutput();
            QByteArray err_now = python_process.readAllStandardError();
            stdout_buffer.append(out_now);
            stderr_buffer.append(err_now);
            full_stdout_accum.append(out_now);
            full_stderr_accum.append(err_now);
            process_stdout_lines(false);
        } else {
            // Even if no data ready, still drain to capture stderr warnings
            QByteArray out_now = python_process.readAllStandardOutput();
            QByteArray err_now = python_process.readAllStandardError();
            if (!out_now.isEmpty() || !err_now.isEmpty()) {
                stdout_buffer.append(out_now);
                stderr_buffer.append(err_now);
                full_stdout_accum.append(out_now);
                full_stderr_accum.append(err_now);
                process_stdout_lines(false);
            }
        }

        // Heartbeat: if >5s since last progress and still below 10%, emit incremental pseudo-progress to show liveness
        qint64 now_ms = QDateTime::currentMSecsSinceEpoch();
        qint64 since_last_progress = now_ms - last_progress_ms;
        if (last_reported < 10 && since_last_progress > 5000) {
            int synthetic = qMin(last_reported + 1, 9);
            emit progressUpdate(job_id_, synthetic);
            last_reported = synthetic;
            last_progress_ms = now_ms;
            qLog(Debug) << "Heartbeat progress emitted (no real PROGRESS lines yet) ->" << synthetic;
        }

        // Check process state
        if (python_process.state() != QProcess::Running) {
            // Drain any remaining output
            stdout_buffer.append(python_process.readAllStandardOutput());
            stderr_buffer.append(python_process.readAllStandardError());
            full_stdout_accum.append(stdout_buffer);
            full_stderr_accum.append(stderr_buffer);
            process_stdout_lines(true);
            break;
        }

        // Timeout check
        qint64 elapsed_ms = now_ms - start_ms;
        if (elapsed_ms > kSeparationTimeoutMs) {
            qLog(Error) << "Python separation timeout (poll loop) (limit ms=" << kSeparationTimeoutMs
                        << ", elapsed ms=" << elapsed_ms << ") job:" << job_id_;
            stdout_buffer.append(python_process.readAllStandardOutput());
            stderr_buffer.append(python_process.readAllStandardError());
            full_stdout_accum.append(stdout_buffer);
            full_stderr_accum.append(stderr_buffer);
            python_process.kill();
            python_process.waitForFinished(3000);
            emit error(job_id_, QString("Separation process timed out after %1 ms (poll loop)").arg(elapsed_ms));
            return;
        }

        msleep(50); // small sleep to reduce CPU usage
    }
    
    if (cancelled_) {
        python_process.kill();
        python_process.waitForFinished(3000);
        qLog(Info) << "Python separation cancelled:" << job_id_;
        return;
    }
    qint64 total_elapsed = QDateTime::currentMSecsSinceEpoch() - start_ms;

    if (python_process.exitCode() == 0) {
        if (last_reported < 100) emit progressUpdate(job_id_, 100);

        // Add small delay to ensure files are fully written
        msleep(500);

        // Find generated stem files
        SeparatedStems stems = findGeneratedStems(job_output_dir);

        if (stems.is_valid) {
            qLog(Info) << "✅ AI separation successful:" << job_id_;
            emit finished(job_id_, stems);
        } else {
            qLog(Error) << "❌ Generated stems not found:" << job_output_dir;
            qLog(Debug) << "Checking output directory contents...";

            // Debug: List directory contents
            QDir debug_dir(job_output_dir);
            QStringList debug_files = debug_dir.entryList(QDir::Files);
            qLog(Debug) << "Files in output directory:" << debug_files;

            emit error(job_id_, "Generated stem files not found");
        }
    } else {
        // Combine stderr + captured diagnostics
        QString combined_err = QString::fromUtf8(full_stderr_accum) + QString::fromUtf8(python_process.readAllStandardError());
        QString combined_out = QString::fromUtf8(full_stdout_accum.left(4000));
        qLog(Error) << "Python separation failed (exitCode=" << python_process.exitCode() << ", elapsed ms=" << total_elapsed << ")";
        qLog(Error) << "Captured stdout (first 4KB):" << combined_out;
        qLog(Error) << "Captured stderr (first 4KB):" << combined_err.left(4000);
        QString error_msg = QString("Python separation failed (code %1)").arg(python_process.exitCode());
        emit error(job_id_, error_msg);
    }
}

void StemSeparationJob::runMockSeparation() {
    qLog(Info) << "Running mock separation (Python not available):" << job_id_;
    
    // Mock progress updates
    for (int i = 10; i <= 90 && !cancelled_; i += 20) {
        msleep(300);
        emit progressUpdate(job_id_, i);
    }
    
    if (!cancelled_) {
        emit progressUpdate(job_id_, 100);
        
        // Create mock result
        SeparatedStems stems;
        stems.is_valid = false; // Mark as invalid since it's mock data
        stems.vocals_file = QString();
        stems.drums_file = QString();
        stems.bass_file = QString();
        stems.other_file = QString();
        
        emit finished(job_id_, stems);
        qLog(Info) << "Mock separation completed:" << job_id_;
    }
}

SeparatedStems StemSeparationJob::findGeneratedStems(const QString& output_dir) {
    SeparatedStems stems;
    
    QDir dir(output_dir);
    QStringList files = dir.entryList(QStringList() << "*.wav" << "*.mp3", QDir::Files);
    
    for (const QString& file : files) {
        QString full_path = dir.absoluteFilePath(file);
        QString lower_file = file.toLower();
        
        if (lower_file.contains("vocals") || lower_file.contains("voice")) {
            stems.vocals_file = full_path;
        } else if (lower_file.contains("drums")) {
            stems.drums_file = full_path;
        } else if (lower_file.contains("bass")) {
            stems.bass_file = full_path;
        } else if (lower_file.contains("other") || lower_file.contains("accomp")) {
            stems.other_file = full_path;
        }
    }
    
    // Check if we have all stems
    stems.is_valid = !stems.vocals_file.isEmpty() && 
                     !stems.drums_file.isEmpty() && 
                     !stems.bass_file.isEmpty() && 
                     !stems.other_file.isEmpty();
    
    stems.timestamp = QDateTime::currentSecsSinceEpoch();
    
    qLog(Info) << "Found stems:" 
               << "vocals=" << !stems.vocals_file.isEmpty()
               << "drums=" << !stems.drums_file.isEmpty()  
               << "bass=" << !stems.bass_file.isEmpty()
               << "other=" << !stems.other_file.isEmpty();
    
    return stems;
}

// ====================================================================================
// LiveStemSeparationJob Implementation
// ====================================================================================

LiveStemSeparationJob::LiveStemSeparationJob(const QString& job_id,
                                           const QString& audio_file,
                                           const QString& model_name,
                                           const QString& output_dir,
                                           const QString& live_script_path,
                                           const QString& python_executable,
                                           QObject* parent)
    : BaseSeparationJob(parent),
      job_id_(job_id),
      audio_file_(audio_file),
      model_name_(model_name),
      output_dir_(output_dir),
      live_script_path_(live_script_path),
      python_executable_(python_executable),
      cancelled_(false) {
}

void LiveStemSeparationJob::run() {
    qLog(Info) << "Starting live separation job:" << job_id_;
    
    try {
        runLivePythonSeparation();
    } catch (const std::exception& e) {
        emit error(job_id_, QString("Live separation exception: %1").arg(e.what()));
    } catch (...) {
        emit error(job_id_, "Unknown error in live separation");
    }
    
    qLog(Info) << "Live separation job completed:" << job_id_;
}

void LiveStemSeparationJob::runLivePythonSeparation() {
    // Check if live script exists
    if (!QFileInfo::exists(live_script_path_)) {
        emit error(job_id_, QString("Live script not found: %1").arg(live_script_path_));
        return;
    }
    
    // Progress update
    emit progressUpdate(job_id_, 5);
    
    // Prepare command
    QStringList arguments;
    arguments << live_script_path_;
    arguments << "--audio" << audio_file_;
    arguments << "--output" << output_dir_;
    arguments << "--model" << model_name_;
    
    qLog(Info) << "Running live Python command with arguments:" << arguments;
    
    // Start the live process
    QProcess python_process;
    python_process.setProgram(python_executable_);
    python_process.setArguments(arguments);
    python_process.setWorkingDirectory(QFileInfo(live_script_path_).absolutePath());
    
    // Connect output handling
    connect(&python_process, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray output = python_process.readAllStandardOutput();
        QString output_str = QString::fromUtf8(output);
        
        for (const QString& line : output_str.split('\n', Qt::SkipEmptyParts)) {
            qLog(Debug) << "Live Python:" << line;
            
            if (line.startsWith("PROGRESS:")) {
                bool ok;
                int progress = line.mid(9).toInt(&ok);
                if (ok && progress >= 0 && progress <= 100) {
                    emit progressUpdate(job_id_, progress);
                }
            } else if (line.startsWith("LIVE_SEPARATION_STARTED")) {
                emit progressUpdate(job_id_, 20);
                qLog(Info) << "Live separation started successfully";
            } else if (line.startsWith("LIVE_STEM:")) {
                // Parse live stem pipe: "LIVE_STEM:drums:/path/to/stem_drums.fifo"
                QStringList parts = line.mid(10).split(':');
                if (parts.size() >= 2) {
                    QString stem_name = parts[0];
                    QString pipe_path = parts[1];
                    qLog(Info) << "Live pipe ready:" << stem_name << "->" << pipe_path;
                }
            }
        }
    });
    
    connect(&python_process, &QProcess::readyReadStandardError, [&]() {
        QByteArray error = python_process.readAllStandardError();
        QString error_str = QString::fromUtf8(error);
        qLog(Warning) << "Live Python stderr:" << error_str;
    });
    
    // Start the process
    python_process.start();
    
    if (!python_process.waitForStarted(10000)) {
        emit error(job_id_, "Failed to start live Python process");
        return;
    }
    
    emit progressUpdate(job_id_, 10);
    
    // Wait for the live process to setup pipes (extended timeout for model loading)
    if (!python_process.waitForFinished(120000)) { // 2 minutes for model loading
        // Process is running in background - this is expected for live mode
        qLog(Info) << "Live process started and running in background";
        
        // Give it a moment to create pipes
        QThread::msleep(1000);
        
        // Create stems structure with pipe paths
        SeparatedStems live_stems = createLivePipeStems(output_dir_);
        
        if (live_stems.is_valid) {
            emit progressUpdate(job_id_, 50);
            emit liveStemsReady(job_id_, live_stems);
            qLog(Info) << "Live stems ready with pipes";
        } else {
            emit error(job_id_, "Failed to create live pipes");
        }
    } else {
        // Process finished quickly - check for errors
        int exit_code = python_process.exitCode();
        if (exit_code != 0) {
            QString error_msg = QString("Live Python process failed with exit code %1").arg(exit_code);
            emit error(job_id_, error_msg);
        }
    }
}

SeparatedStems LiveStemSeparationJob::createLivePipeStems(const QString& output_dir) {
    SeparatedStems stems;
    
    // Expected pipe names from the live Python script
    QStringList pipe_names = {"drums", "bass", "other", "vocals"};
    QStringList pipe_paths;
    
    QDir dir(output_dir);
    bool all_pipes_exist = true;
    
    for (const QString& stem_name : pipe_names) {
        QString pipe_path = dir.absoluteFilePath(QString("stem_%1.fifo").arg(stem_name));
        
        if (QFileInfo::exists(pipe_path)) {
            pipe_paths.append(pipe_path);
            qLog(Info) << "Found live pipe:" << pipe_path;
        } else {
            qLog(Warning) << "Live pipe not found:" << pipe_path;
            all_pipes_exist = false;
        }
    }
    
    if (all_pipes_exist && pipe_paths.size() == 4) {
        // Assign pipe paths to stems (matching the order from live script)
        stems.drums_file = pipe_paths[0];   // drums
        stems.bass_file = pipe_paths[1];    // bass  
        stems.other_file = pipe_paths[2];   // other
        stems.vocals_file = pipe_paths[3];  // vocals
        
        stems.is_valid = true;
        stems.timestamp = QDateTime::currentSecsSinceEpoch();
        
        qLog(Info) << "Live stems configured:"
                   << "drums=" << stems.drums_file
                   << "bass=" << stems.bass_file
                   << "other=" << stems.other_file
                   << "vocals=" << stems.vocals_file;
    } else {
        stems.is_valid = false;
        qLog(Error) << "Failed to setup live pipes - not all pipes created";
    }
    
    return stems;
}
