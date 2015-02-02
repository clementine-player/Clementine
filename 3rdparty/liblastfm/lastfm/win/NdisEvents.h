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
#ifndef NDIS_EVENTS_H
#define NDIS_EVENTS_H

#include <windows.h>

#ifndef __MINGW32__
#include <atlbase.h>
#include <WbemCli.h>
#endif

class NdisEvents
{
public:
    NdisEvents();
    ~NdisEvents();
    HRESULT registerForNdisEvents();

    virtual void onConnectionUp(BSTR name) = 0;
    virtual void onConnectionDown(BSTR name) = 0;

private:
    CComPtr<IWbemLocator> m_pLocator;
    CComPtr<IWbemServices> m_pServices;
    class WmiSink *m_pSink;
};

#endif

