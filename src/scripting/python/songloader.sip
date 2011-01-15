class SongLoader : QObject {

%TypeHeaderCode
  #include "core/songloader.h"
%End

public:
  SongLoader(LibraryBackend* library, QObject* parent /TransferThis/ = 0);

  enum Result {
    Success,
    Error,
    WillLoadAsync,
  };

  static const int kDefaultTimeout;

  const QUrl& url() const;
  const SongList& songs() const;

  int timeout() const;
  void set_timeout(int msec);

  Result Load(const QUrl& url);

signals:
  void LoadFinished(bool success);
};
