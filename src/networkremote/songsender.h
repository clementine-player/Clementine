#ifndef SONGSENDER_H
#define SONGSENDER_H

#include <QMap>
#include <QQueue>
#include <QUrl>

#include "remotecontrolmessages.pb.h"

#include "core/song.h"
#include "transcoder/transcoder.h"

class Application;
class RemoteClient;
class Transcoder;

struct DownloadItem {
  Song song_;
  int song_no_;
  int song_count_;
  DownloadItem(Song s, int no, int count)
    : song_(s), song_no_(no), song_count_(count) {}
};

class SongSender : public QObject {
  Q_OBJECT
 public:
  SongSender(Application* app, RemoteClient* client);

  static const quint32 kFileChunkSize;

 public slots:
  void SendSongs(const pb::remote::RequestDownloadSongs& request);
  void ResponseSongOffer(bool accepted);

 private slots:
  void TranscodeJobComplete(const QString& input, const QString& output, bool success);
  void StartTransfer();

 private:
  Application* app_;
  RemoteClient* client_;

  TranscoderPreset transcoder_preset_;
  Transcoder* transcoder_;
  bool transcode_lossless_files_;

  QQueue<DownloadItem> download_queue_;
  QMap<QString, QString> transcoder_map_;
  int total_transcode_;

  void SendSingleSong(DownloadItem download_item);
  void SendAlbum(const Song& song);
  void SendPlaylist(int playlist_id);
  void SendUrls(const pb::remote::RequestDownloadSongs& request);
  void OfferNextSong();
  void SendTotalFileSize();
  void TranscodeLosslessFiles();
  void SendTranscoderStatus();
};

#endif  // SONGSENDER_H
