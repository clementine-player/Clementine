class NetworkAccessManager : QNetworkAccessManager {

%TypeHeaderCode
#include "core/network.h"
%End

%Docstring
A Qt network access manager that should be used to create all network requests
in Clementine.

Differences from the normal QNetworkAccessManager:

  - Sends a Clementine C{User-Agent} HTTP header.
  - Uses a thread-safe disk cache in Clementine's user config directory.
  - Honors the proxy set by the user in the settings dialog.
%End

public:
  NetworkAccessManager(QObject* parent /TransferThis/ = 0);
};
