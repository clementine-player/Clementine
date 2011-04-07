#include "extensions.h"

namespace xrme {

const char* MediaPlayerExtension::kFilterString =
    "/iq/xrme[@xmlns='http://purplehatstands.com/xmlns/xrme/mediaplayer']";
const char* RemoteControlExtension::kFilterString =
    "/iq/xrme[@xmlns='http://purplehatstands.com/xmlns/xrme/remotecontrol']";
const char* MediaStorageExtension::kFilterString =
    "/iq/xrme[@xmlns='http://purplehatstands.com/xmlns/xrme/mediastorage']";
const char* SIPNegotiation::kFilterString =
    "/iq/xrme[@xmlns='http://purplehatstands.com/xmlns/xrme/sipnegotiation']";

}
