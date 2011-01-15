class NetworkAccessManager : QNetworkAccessManager {

%TypeHeaderCode
#include "core/network.h"
%End

public:
  NetworkAccessManager(QObject* parent /TransferThis/ = 0);
};
