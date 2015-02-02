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
#include <windows.h>
#include <winhttp.h>


/** @brief memory managing wrapper for WINHTTP_CURRENT_USER_IE_PROXY_CONFIG
  * @author <doug@last.fm>
  */
struct IeSettings : WINHTTP_CURRENT_USER_IE_PROXY_CONFIG
{
    IeSettings()
    {
        if (!WinHttpGetIEProxyConfigForCurrentUser(this)) {
            fAutoDetect = FALSE;
            lpszAutoConfigUrl = lpszProxy = lpszProxyBypass = 0;
        }
    }
    
    ~IeSettings()
    {
        if (lpszAutoConfigUrl) GlobalFree(lpszAutoConfigUrl);
        if (lpszProxy) GlobalFree(lpszProxy);
        if (lpszProxyBypass) GlobalFree(lpszProxyBypass);
    }
};
