#ifndef SONGSENDER_H
#define SONGSENDER_H

#include <QFile>
#include <QHash>
#include <QMap>
#include <QQueue>
#include <QUrl>
#include <memory>

#include "core/song.h"
#include "remotecontrolmessages.pb.h"
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
  ~SongSender();

  static const quint32 kFileChunkSize;

 public slots:
  void SendSongs(const cpb::remote::RequestDownloadSongs& request,
                 const Song& current_song = Song(), int current_row = -1,
                 const SongList& playlist_songs = SongList());
  void ResponseSongOffer(bool accepted);

 private slots:
  void TranscodeJobComplete(const QUrl& input, const QString& output,
                            bool success);
  void StartTransfer();
  void PumpNextChunk();

 private:
  Application* app_;
  RemoteClient* client_;

  TranscoderPreset transcoder_preset_;
  bool has_transcoder_preset_;
  Transcoder* transcoder_;
  bool transcode_lossless_files_;

  QQueue<DownloadItem> download_queue_;
  QMap<QString, QString> transcoder_map_;
  QHash<QString, int> transcode_refs_;
  int total_transcode_;
  int transcode_processed_;
  int current_row_;

  std::unique_ptr<QFile> active_file_;
  DownloadItem active_item_;
  QString active_source_path_;
  QByteArray active_sha1_;
  qint64 active_file_size_;
  int active_chunk_count_;
  int active_chunk_number_;
  bool active_is_transcoded_;

  void SendSingleSong(const DownloadItem& download_item);
  void FinishActiveTransfer(bool offer_next);
  void ReleaseTranscodedRef(const QString& local_file);
  void CleanupTranscodedFiles();
  void SendAlbum(const Song& song);
  void SendPlaylist(const cpb::remote::RequestDownloadSongs& request,
                    const SongList& song_list);
  void SendUrls(const cpb::remote::RequestDownloadSongs& request);
  void OfferNextSong();
  void SendTotalFileSize();
  void TranscodeLosslessFiles();
  void SendTranscoderStatus();
};

#endif  // SONGSENDER_H
