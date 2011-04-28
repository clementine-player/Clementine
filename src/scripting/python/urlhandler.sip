class UrlHandler : QObject {

%TypeHeaderCode
#include "core/urlhandler.h"
%End

public:
  UrlHandler(QObject* parent /TransferThis/  = 0);

  // The URL scheme that this handler handles.
  virtual QString scheme() const = 0;

  struct LoadResult {
%Docstring
Returned by StartLoading() and LoadNext(), indicates what the player should do
when it wants to load a URL.

Valid values for the type_ field are:

  - C{NoMoreTracks} - there wasn't a track available, and the player should
    move on to the next playlist item.
  - C{WillLoadAsynchronously} - there might be another track available but the
    handler needs to do some work (eg. fetching a remote playlist) to find out.
    AsyncLoadComplete will be emitted later with the same original_url.
  - C{TrackAvailable} - There was a track available.  Its url is in media_url.

%End
    enum Type {
      NoMoreTracks,
      WillLoadAsynchronously,
      TrackAvailable,
    };

    LoadResult(const QUrl& original_url = QUrl(),
               Type type = NoMoreTracks,
               const QUrl& media_url = QUrl());

    QUrl original_url_;
    Type type_;
    QUrl media_url_;
  };

  virtual LoadResult StartLoading(const QUrl& url);
  virtual LoadResult LoadNext(const QUrl& url);

signals:
  void AsyncLoadComplete(const UrlHandler::LoadResult& result);
};
