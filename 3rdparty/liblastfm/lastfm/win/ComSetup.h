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
#ifndef _WIN32_WINNT
// This means we are targetting Windows XP onwards
// bring in CoInitializeSecurity from objbase.h
#define _WIN32_WINNT 0x0501
#endif

#include <objbase.h>

#ifndef __MINGW32__
    #include <atlbase.h>
    #include <atlcom.h>
#endif


/** @brief WsConnectionMonitor needs Com to work as early as possible so we do this
  * @author <doug@last.fm> 
  */
class ComSetup
{
public:
    ComSetup()
    {
        HRESULT hr = CoInitialize(0);
        m_bComInitialised = SUCCEEDED(hr);
        _ASSERT(m_bComInitialised);
        if (m_bComInitialised) {
            setupSecurity();
        }
    }
    
    void setupSecurity()
    {
        CSecurityDescriptor sd;
        sd.InitializeFromThreadToken();
        HRESULT hr = CoInitializeSecurity(sd, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL); 
        _ASSERT(SUCCEEDED(hr));
    }
    
    ~ComSetup()
    {
        if (m_bComInitialised) {
            CoUninitialize();
        }
    }
    
private:
    bool m_bComInitialised;
};
