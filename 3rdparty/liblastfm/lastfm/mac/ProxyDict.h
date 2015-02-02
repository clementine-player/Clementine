/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <SystemConfiguration/SystemConfiguration.h>

struct ProxyDict
{
    ProxyDict();

    int port;
    QString host;
    
    bool isProxyEnabled() const { return port > 0 && host.size(); }
};


inline ProxyDict::ProxyDict() : port( 0 )
{
    // Get the dictionary.
    CFDictionaryRef proxyDict = SCDynamicStoreCopyProxies( NULL );
    bool result = (proxyDict != NULL);

    // Get the enable flag.  This isn't a CFBoolean, but a CFNumber.
    CFNumberRef enableNum;
    int enable;
    if (result) {
        enableNum = (CFNumberRef) CFDictionaryGetValue( proxyDict, kSCPropNetProxiesHTTPEnable );
        result = (enableNum != NULL) && (CFGetTypeID(enableNum) == CFNumberGetTypeID());
    }
    if (result)
        result = CFNumberGetValue( enableNum, kCFNumberIntType, &enable ) && (enable != 0);

    // Get the proxy host.  DNS names must be in ASCII.  If you 
    // put a non-ASCII character  in the "Secure Web Proxy"
    // field in the Network preferences panel, the CFStringGetCString
    // function will fail and this function will return false.
    CFStringRef hostStr;
    if (result) {
        hostStr = (CFStringRef) CFDictionaryGetValue( proxyDict, kSCPropNetProxiesHTTPProxy );
        result = (hostStr != NULL) && (CFGetTypeID(hostStr) == CFStringGetTypeID());
    }
    if (result)
        host = lastfm::CFStringToQString( hostStr );

    // get the proxy port
    CFNumberRef portNum;

    if (result) {
        portNum = (CFNumberRef) CFDictionaryGetValue( proxyDict, kSCPropNetProxiesHTTPPort );
        result = (portNum != NULL) && (CFGetTypeID(portNum) == CFNumberGetTypeID());
    }
    if (result)
        result = CFNumberGetValue( portNum, kCFNumberIntType, &port );

    // clean up.
    if (proxyDict != NULL)
        CFRelease( proxyDict );
}
