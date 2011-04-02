class CoverProvider : QObject {

%TypeHeaderCode
#include "covers/albumcoverfetcher.h"
#include "covers/coverprovider.h"
%End

%Docstring
Each implementation of this interface downloads covers from one online
service.

There are no limitations on what this service might be - last.fm, Amazon,
Google Images - you name it.

CoverProvider should be stateless since it will be used in multi-threaded
environment. It can and probably should use some contextual information
though because a single search spans through two interface methods invoked
independently. The key to connecting both invocations is the NetworkReply
object.

A flow of single request for covers:
- Provider is asked to prepare and invoke a NetworkRequest (L{SendRequest()})
  for a given cover query. Provider then returns a corresponding NetworkReply.
  It should not connect to finished() signal of the reply, though!
- Later, in a separate invocation (L{ParseReply()}), provider will be asked to
  parse the NetworkReply it once prepared. The result is a QList of L{CoverSearchResult}
  objects.

Every CoverProvider has a name which should describe (in a word or two) the service
it's using.
%End

public:
  CoverProvider(const QString& name);
  virtual ~CoverProvider();

  QString name();
%Docstring
Name of this provider, like "last.fm".
%End

  virtual QNetworkReply* SendRequest(const QString& query) = 0;
%Docstring
Given a search request from Clementine, provider has to create and invoke
a NetworkRequest.

It then has to return a corresponding NetworkReply, without connecting to
it's finished() signal!
%End

  virtual CoverSearchResults ParseReply(QNetworkReply* reply) = 0;
%Docstring
Provider parses a reply which is now filled with data obtained from a service
this provider communicates with. The result is a QList of L{CoverSearchResult}
objects.
%End

};
