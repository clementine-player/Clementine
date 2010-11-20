/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wmdmprogress.h"

#include <QtDebug>

WmdmProgress::WmdmProgress(const MusicStorage::ProgressFunction& f)
  : f_(f),
    estimated_(0)
{
}

LONG WmdmProgress::QueryInterface(const IID& riid, void** object) {
  *object = 0;

  if (riid == IID_IUnknown)
    *object = (IUnknown*) this;
  else if (riid == IID_IWMDMProgress)
    *object = (IWMDMProgress*) this;
  else if (riid == IID_IWMDMProgress2)
    *object = (IWMDMProgress2*) this;
  else if (riid == IID_IWMDMProgress3)
    *object = (IWMDMProgress3*) this;
  else
    return E_NOINTERFACE;

  return S_OK;
}

ULONG WmdmProgress::AddRef() {
  return 0;
}

ULONG WmdmProgress::Release() {
  return 0;
}

HRESULT WmdmProgress::Begin(DWORD estimated_ticks) {
  return Begin3(EVENT_WMDM_CONTENT_TRANSFER, estimated_ticks, NULL);
}

HRESULT WmdmProgress::End() {
  return End3(EVENT_WMDM_CONTENT_TRANSFER, S_OK, NULL);
}

HRESULT WmdmProgress::Progress(DWORD transpired_ticks) {
  return Progress3(EVENT_WMDM_CONTENT_TRANSFER, transpired_ticks, NULL);
}

HRESULT WmdmProgress::End2(HRESULT completion_code) {
  return End3(EVENT_WMDM_CONTENT_TRANSFER, completion_code, NULL);
}

HRESULT WmdmProgress::Begin3(GUID, DWORD estimated_ticks, OPAQUECOMMAND*) {
  estimated_ = estimated_ticks;
  return S_OK;
}

HRESULT WmdmProgress::End3(GUID, HRESULT, OPAQUECOMMAND*) {
  return S_OK;
}

HRESULT WmdmProgress::Progress3(GUID, DWORD transpired_ticks, OPAQUECOMMAND*) {
  if (estimated_ != 0)
    f_(float(transpired_ticks) / estimated_);
  return S_OK;
}
