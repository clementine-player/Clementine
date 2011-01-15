class ParserBase : QObject {
%TypeHeaderCode
  #include "playlistparsers/parserbase.h"
%End

public:
  ParserBase(LibraryBackend* library, QObject* parent /TransferThis/ = 0);

  virtual QString name() const = 0;
  virtual QStringList file_extensions() const = 0;
  virtual QString mime_type() const;

  virtual bool TryMagic(const QByteArray& data) const = 0;

  virtual SongList Load(QIODevice* device, const QString& playlist_path = "", const QDir& dir = QDir()) const = 0;
  virtual void Save(const SongList& songs, QIODevice* device, const QDir& dir = QDir()) const = 0;
};
