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
#include "NdisEvents.h"
#include "WmiSink.h"

// see http://msdn.microsoft.com/en-us/magazine/cc301850.aspx for
// more about Ndis and wmi and getting these events

// Link to wbemuuid.lib to resolve IWbemObjectSink and IWbemClassObject
// interface definitions.

NdisEvents::NdisEvents()
    : m_pSink(0)
{}

NdisEvents::~NdisEvents()
{
    if (m_pSink)
        m_pSink->disconnect();
    if (m_pServices && m_pSink)
        m_pServices->CancelAsyncCall(m_pSink);
    // and reference counting will take care of the WmiSink object
}

HRESULT
NdisEvents::registerForNdisEvents()
{
    HRESULT hr = m_pLocator.CoCreateInstance(CLSID_WbemLocator);
    if (FAILED(hr))
        return hr;

    // Connect to the root\wmi namespace with the current user.
    hr = m_pLocator->ConnectServer(CComBSTR("ROOT\\WMI"),       // strNetworkResource
                                    NULL,               // strUser
                                    NULL,               // strPassword
                                    NULL,               // strLocale  
                                    0,                  // lSecurityFlags
                                    CComBSTR(""),       // strAuthority               
                                    NULL,               // pCtx
                                    &m_pServices
                                    );
    if (FAILED(hr))
        return hr;

    m_pSink = new WmiSink(this);

    //////////////////////////

    // other notifications we're not interested in right now include...
    // MSNdis_NotifyAdapterArrival  \DEVICE\<guid>
    // MSNdis_NotifyAdapterRemoval
    // MSNdis_StatusLinkSpeedChange
    // MSNdis_NotifyVcArrival
    // MSNdis_NotifyVcRemoval
    // MSNdis_StatusResetStart
    // MSNdis_StatusResetEnd
    // MSNdis_StatusProtocolBind
    // MSNdis_StatusProtocolUnbind
    // MSNdis_StatusMediaSpecificIndication

    CComBSTR wql("WQL");
    CComBSTR query("SELECT * FROM MSNdis_StatusMediaDisconnect");
    hr = m_pServices->ExecNotificationQueryAsync(wql, query, 0, 0, m_pSink);

    query = "SELECT * FROM MSNdis_StatusMediaConnect";
    hr = m_pServices->ExecNotificationQueryAsync(wql, query, 0, 0, m_pSink);

    return S_OK;
}

