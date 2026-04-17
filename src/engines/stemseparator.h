#ifndef STEMSEPARATOR_H
#define STEMSEPARATOR_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QThread>
#include <QMutex>
#include <QMetaType>

// Forward declarations
class StemSeparationJob;
class BaseSeparationJob;

struct SeparatedStems {
    enum PlayMode {
        AllStems = 0,    // Original track or mixed stems
        VocalsOnly = 1,  // Only vocals stem
        Instrumental,    // All except vocals
        DrumsOnly,       // Only drums stem
        BassOnly,        // Only bass stem
        OtherOnly        // Only other instruments stem
    };
    
    QString vocals_file;
    QString drums_file;
    QString bass_file;
    QString other_file;
    bool is_valid = false;
    qint64 timestamp = 0;
};

// Register SeparatedStems as Qt metatype for signal/slot connections
Q_DECLARE_METATYPE(SeparatedStems)

/**
 * @brief AI-powered stem separation for Clementine Extended
 * 
 * This class provides the main interface for separating audio into stems
 * using Facebook's Demucs model. It supports both offline and cached processing.
 */
class StemSeparator : public QObject {
    Q_OBJECT

public:
    enum ProcessingState {
        Idle,
        Processing,
        Completed,
        Error
    };

    explicit StemSeparator(QObject* parent = nullptr);
    ~StemSeparator();

    // Main interface
    bool initialize();
    void cleanup();
    
    // Separation control
    QString startSeparation(const QString& audio_file);
    QString startLiveSeparation(const QString& audio_file);  // New: Start live streaming separation
    ProcessingState getProcessingState(const QString& job_id) const;
    SeparatedStems getResults(const QString& job_id) const;
    void cancelSeparation(const QString& job_id);
    
    // Cache management
    bool hasCachedStems(const QString& audio_file) const;
    SeparatedStems getCachedStems(const QString& audio_file) const;
    void clearCache();
    
    // Configuration
    void setModelName(const QString& model = "htdemucs");
    void setOutputDirectory(const QString& dir);
    void setCacheEnabled(bool enabled);
    
    // Python integration status
    bool isPythonAvailable() const { return python_available_; }
    QString getPythonScriptPath() const { return python_script_path_; }
    void setMaxCacheSize(qint64 max_size_mb);
    
    // Status information
    bool isInitialized() const { return initialized_; }
    QString getLastError() const { return last_error_; }
    QStringList getAvailableModels() const;

public slots:
    void onSeparationFinished(const QString& job_id, const SeparatedStems& stems);
    void onSeparationError(const QString& job_id, const QString& error);
    void onProgressUpdate(const QString& job_id, int progress);

signals:
    void separationStarted(const QString& job_id);
    void separationFinished(const QString& job_id, const SeparatedStems& stems);
    void separationProgress(const QString& job_id, int progress);
    void separationError(const QString& job_id, const QString& error);
    void liveStemsReady(const QString& job_id, const SeparatedStems& stems);  // New: Live streams ready
    void cacheUpdated();

private slots:
    void cleanupFinishedJobs();

private:
    // Core functionality
    bool initializePythonEnvironment();
    bool checkPythonEnvironment();
    QString generateJobId() const;
    QString getCacheKey(const QString& audio_file) const;
    void saveToCache(const QString& cache_key, const SeparatedStems& stems);
    void cleanupOldCacheEntries();
    
    // Member variables
    bool initialized_;
    QString model_name_;
    QString output_directory_;
    QString cache_directory_;
    QString last_error_;
    
    // Job management
    QHash<QString, BaseSeparationJob*> active_jobs_;  // Use base class for polymorphism
    QHash<QString, SeparatedStems> completed_jobs_;
    mutable QMutex jobs_mutex_;
    
    // Cache management
    bool cache_enabled_;
    qint64 max_cache_size_mb_;
    QHash<QString, SeparatedStems> cache_;
    mutable QMutex cache_mutex_;
    
    // Python integration
    void* python_module_;  // PyObject* 
    QString python_script_path_;
    QString python_executable_;  // Path to Python executable (venv or system)
    bool python_available_; 
    void* python_separator_class_;  // PyObject*
    void* python_instance_;  // PyObject*
};

/**
 * @brief Base class for all separation jobs
 */
class BaseSeparationJob : public QThread {
    Q_OBJECT

public:
    explicit BaseSeparationJob(QObject* parent = nullptr) : QThread(parent) {}
    virtual ~BaseSeparationJob() = default;
    
    virtual void cancel() = 0;
    virtual bool isCancelled() const = 0;
    virtual const QString& getJobId() const = 0;

signals:
    void progressUpdate(const QString& job_id, int progress);
    void error(const QString& job_id, const QString& error);
};

/**
 * @brief Background worker thread for stem separation
 */
class StemSeparationJob : public BaseSeparationJob {
    Q_OBJECT

public:
    StemSeparationJob(const QString& job_id, 
                      const QString& audio_file,
                      const QString& model_name,
                      const QString& output_dir,
                      QObject* parent = nullptr);

    const QString& getJobId() const override { return job_id_; }
    const QString& getAudioFile() const { return audio_file_; }
    void cancel() override { cancelled_ = true; }
    bool isCancelled() const override { return cancelled_; }

protected:
    void run() override;

private:
    void runPythonSeparation();
    void runMockSeparation();
    SeparatedStems findGeneratedStems(const QString& output_dir);

signals:
    void finished(const QString& job_id, const SeparatedStems& stems);

private:
    QString job_id_;
    QString audio_file_;
    QString model_name_;
    QString output_dir_;
    bool cancelled_;
};

/**
 * @brief Live streaming stem separation job
 * Creates named pipes for real-time audio streaming
 */
class LiveStemSeparationJob : public BaseSeparationJob {
    Q_OBJECT

public:
    LiveStemSeparationJob(const QString& job_id, 
                         const QString& audio_file,
                         const QString& model_name,
                         const QString& output_dir,
                         const QString& live_script_path,
                         const QString& python_executable,
                         QObject* parent = nullptr);

    const QString& getJobId() const override { return job_id_; }
    const QString& getAudioFile() const { return audio_file_; }
    void cancel() override { cancelled_ = true; }
    bool isCancelled() const override { return cancelled_; }

protected:
    void run() override;

private:
    void runLivePythonSeparation();
    SeparatedStems createLivePipeStems(const QString& output_dir);

signals:
    void liveStemsReady(const QString& job_id, const SeparatedStems& stems);

private:
    QString job_id_;
    QString audio_file_;
    QString model_name_;
    QString output_dir_;
    QString live_script_path_;
    QString python_executable_;
    bool cancelled_;
};

#endif // STEMSEPARATOR_H
