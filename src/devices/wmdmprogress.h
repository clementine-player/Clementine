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

#ifndef WMDMPROGRESS_H
#define WMDMPROGRESS_H

#include <mswmdm.h>

#include "core/musicstorage.h"

class WmdmProgress : public IWMDMProgress3 {
public:
  WmdmProgress(const MusicStorage::ProgressFunction& f =
                     MusicStorage::ProgressFunction());

  // IUnknown
  // The __stdcall is *really* important
  virtual LONG __stdcall QueryInterface(const IID& riid, void** object);
  virtual ULONG __stdcall AddRef();
  virtual ULONG __stdcall Release();

  // IWMDMProgress
  virtual HRESULT __stdcall Begin(DWORD estimated_ticks);
  virtual HRESULT __stdcall End();
  virtual HRESULT __stdcall Progress(DWORD transpired_ticks);

  // IWMDMProgress2
  virtual HRESULT __stdcall End2(HRESULT completion_code);

  // IWMDMProgress3
  virtual HRESULT __stdcall Begin3(GUID event_id, DWORD estimated_ticks,
                                   OPAQUECOMMAND* context);
  virtual HRESULT __stdcall End3(GUID event_id, HRESULT completion_code,
                                 OPAQUECOMMAND* context);
  virtual HRESULT __stdcall Progress3(GUID event_id, DWORD transpired_ticks,
                                      OPAQUECOMMAND* context);

private:
  MusicStorage::ProgressFunction f_;

  DWORD estimated_;
};

#endif // WMDMPROGRESS_H
