class PlaylistParser : QObject {
%TypeHeaderCode
  #include "playlistparsers/playlistparser.h"
%End

public:
  PlaylistParser(LibraryBackend* library, QObject* parent /TransferThis/ = 0);

  static const int kMagicSize;

  QStringList file_extensions() const;
  QString filters() const;

  QString default_extension() const;
  QString default_filter() const;

  ParserBase* MaybeGetParserForMagic(const QByteArray& data,
                                     const QString& mime_type = QString()) const;
  ParserBase* ParserForExtension(const QString& suffix) const;

  SongList Load(const QString& filename, const QString& playlist_path = "", ParserBase* parser = 0) const;
  SongList Load(QIODevice* device, const QString& path_hint = "", const QDir& dir_hint = QDir()) const;
  void Save(const SongList& songs, const QString& filename) const;
};
