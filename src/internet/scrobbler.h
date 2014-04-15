#ifndef SCROBBLER_H
#define SCROBBLER_H

#include <QObject>

class Song;

class Scrobbler : public QObject {
  Q_OBJECT

 public:
  Scrobbler(QObject* parent = nullptr) {}

  virtual bool IsAuthenticated() const = 0;
  virtual bool IsScrobblingEnabled() const = 0;
  virtual bool AreButtonsVisible() const = 0;
  virtual bool IsScrobbleButtonVisible() const = 0;
  virtual bool PreferAlbumArtist() const = 0;

 public slots:
  virtual void NowPlaying(const Song& song) = 0;
  virtual void Scrobble() = 0;
  virtual void Love() = 0;
  virtual void ToggleScrobbling() = 0;
  virtual void ShowConfig() = 0;

signals:
  void AuthenticationComplete(bool success, const QString& error_message);
  void ScrobblingEnabledChanged(bool value);
  void ButtonVisibilityChanged(bool value);
  void ScrobbleButtonVisibilityChanged(bool value);
  void ScrobbleSubmitted();
  void ScrobbleError(int value);
};

#endif  // SCROBBLER_H
