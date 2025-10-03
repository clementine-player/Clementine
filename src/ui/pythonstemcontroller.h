#ifndef PYTHONSTEMCONTROLLER_H
#define PYTHONSTEMCONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>

/**
 * @brief Controller for Python stem player backend
 * 
 * This class manages communication with the Python stem separation
 * and playback process, providing a Qt interface for stem control.
 */
class PythonStemController : public QObject {
    Q_OBJECT

public:
    enum PlaybackState {
        STOPPED = 0,
        PLAYING = 1,
        PAUSED = 2
    };

    explicit PythonStemController(QObject* parent = nullptr);
    ~PythonStemController();

    // Main control methods
    bool startPlayer(const QString& audio_file, const QString& output_dir);
    void stopPlayer();
    
    // Playback controls
    bool play();
    bool pause();
    bool resume();
    bool stop();
    
    // Stem controls
    bool setStemVolume(int stem_index, float volume);
    bool setStemMute(int stem_index, bool mute);
    bool setStemSolo(int stem_index, bool solo);
    
    // Status queries
    PlaybackState getPlaybackState() const { return playback_state_; }
    float getPosition() const { return position_; }
    float getDuration() const { return duration_; }
    float getProgress() const { return progress_; }
    bool isReady() const { return is_ready_; }
    
    // Stem state queries
    float getStemVolume(int stem_index) const;
    bool isStemMute(int stem_index) const;
    bool isStemSolo(int stem_index) const;

public slots:
    void requestStatus();

signals:
    void playerReady();
    void playerError(const QString& error);
    void playbackStateChanged(PlaybackState state);
    void positionChanged(float position);
    void progressChanged(float progress);
    void stemVolumeChanged(int stem_index, float volume);
    void stemMuteChanged(int stem_index, bool mute);
    void stemSoloChanged(int stem_index, bool solo);

private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exit_code, QProcess::ExitStatus exit_status);
    void onProcessError(QProcess::ProcessError error);
    void onStatusTimer();

private:
    bool sendCommand(const QJsonObject& command);
    void handleResponse(const QString& response);
    void updateStatusFromJson(const QJsonObject& status);
    
    QProcess* python_process_;
    QTimer* status_timer_;
    
    // State tracking
    bool is_ready_;
    PlaybackState playback_state_;
    float position_;
    float duration_;
    float progress_;
    
    // Stem state
    float stem_volumes_[4];
    bool stem_mutes_[4];
    bool stem_solos_[4];
    
    // Process info
    QString python_script_path_;
    QString current_audio_file_;
    QString current_output_dir_;
};

#endif // PYTHONSTEMCONTROLLER_H
