/* This file is part of Clementine.
   Copyright 2026, David Sansome <me@davidsansome.com>

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

#include "windowsmediacontrols.h"

// windows.h must come before the other Windows headers.
// clang-format off
#include <windows.h>
// clang-format on

#include <propkey.h>
#include <propsys.h>
#include <propvarutil.h>
#include <roapi.h>
#include <shcore.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <systemmediatransportcontrolsinterop.h>
#include <windows.media.h>
#include <windows.storage.streams.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <QBuffer>
#include <QImage>
#include <QMetaObject>
#include <QMutex>
#include <atomic>

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/song.h"
#include "covers/currentartloader.h"
#include "engines/enginebase.h"
#include "playlist/playlistmanager.h"

using ABI::Windows::Foundation::ITypedEventHandler;
using ABI::Windows::Media::IMusicDisplayProperties;
using ABI::Windows::Media::IMusicDisplayProperties2;
using ABI::Windows::Media::ISystemMediaTransportControls;
using ABI::Windows::Media::ISystemMediaTransportControlsButtonPressedEventArgs;
using ABI::Windows::Media::ISystemMediaTransportControlsDisplayUpdater;
using ABI::Windows::Media::SystemMediaTransportControls;
using ABI::Windows::Media::SystemMediaTransportControlsButton;
using ABI::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs;
using ABI::Windows::Storage::Streams::IRandomAccessStream;
using ABI::Windows::Storage::Streams::IRandomAccessStreamReference;
using ABI::Windows::Storage::Streams::IRandomAccessStreamReferenceStatics;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Wrappers::HString;

namespace {

const wchar_t kWindowClassName[] = L"ClementineMediaControls";

// Windows gets the name and icon shown in the media flyout from the Start menu
// shortcut with the same AppUserModelID as the window.  The installer sets this
// ID on its shortcut (see dist/windows/AppUserModelId.nsh).  Without a matching
// shortcut the flyout shows "Unknown app", but the controls still work.
const wchar_t kAppUserModelId[] = L"Clementine.Clementine";

// Album art is shown small in the volume flyout, so don't hand SMTC a huge
// image.
const int kThumbnailSize = 300;

typedef ITypedEventHandler<SystemMediaTransportControls*,
                           SystemMediaTransportControlsButtonPressedEventArgs*>
    ButtonPressedHandlerInterface;

// Calls setter with an HSTRING holding the given QString.
template <typename Setter>
HRESULT PutString(const QString& str, Setter setter) {
  HString hstring;
  HRESULT hr =
      hstring.Set(reinterpret_cast<const wchar_t*>(str.utf16()), str.length());
  if (FAILED(hr)) return hr;
  return setter(hstring.Get());
}

// Sets (or with nullptr, clears) the AppUserModelID of a window.
void SetWindowAppUserModelId(HWND hwnd, const wchar_t* id) {
  ComPtr<IPropertyStore> store;
  if (FAILED(SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(&store)))) {
    return;
  }

  PROPVARIANT value;
  if (id) {
    if (FAILED(InitPropVariantFromString(id, &value))) return;
  } else {
    PropVariantInit(&value);
  }
  store->SetValue(PKEY_AppUserModel_ID, value);
  store->Commit();
  PropVariantClear(&value);
}

}  // namespace

// MinGW's WRL doesn't have Microsoft::WRL::Callback, so the event handler is
// implemented as a COM object by hand.  SMTC invokes it on a thread pool
// thread, so it posts the button to the GUI thread.
class WindowsMediaControls::ButtonPressedHandler
    : public ButtonPressedHandlerInterface,
      public IAgileObject {
 public:
  explicit ButtonPressedHandler(WindowsMediaControls* target)
      : ref_count_(1), target_(target) {}

  // Stops any further events being forwarded to the target.  Once this
  // returns it's safe to delete the target.
  void Detach() {
    QMutexLocker l(&mutex_);
    target_ = nullptr;
  }

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                           void** object) override {
    if (!object) return E_POINTER;
    if (riid == __uuidof(IUnknown) ||
        riid == __uuidof(ButtonPressedHandlerInterface)) {
      *object = static_cast<ButtonPressedHandlerInterface*>(this);
    } else if (riid == __uuidof(IAgileObject)) {
      *object = static_cast<IAgileObject*>(this);
    } else {
      *object = nullptr;
      return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
  }

  ULONG STDMETHODCALLTYPE AddRef() override { return ++ref_count_; }

  ULONG STDMETHODCALLTYPE Release() override {
    ULONG count = --ref_count_;
    if (count == 0) delete this;
    return count;
  }

  // ITypedEventHandler
  HRESULT STDMETHODCALLTYPE
  Invoke(ISystemMediaTransportControls* /* sender */,
         ISystemMediaTransportControlsButtonPressedEventArgs* args) override {
    SystemMediaTransportControlsButton button;
    HRESULT hr = args->get_Button(&button);
    if (FAILED(hr)) return hr;

    QMutexLocker l(&mutex_);
    if (target_) {
      QMetaObject::invokeMethod(target_, "ButtonPressed", Qt::QueuedConnection,
                                Q_ARG(int, button));
    }
    return S_OK;
  }

 private:
  virtual ~ButtonPressedHandler() {}

  std::atomic<ULONG> ref_count_;
  QMutex mutex_;
  WindowsMediaControls* target_;
};

struct WindowsMediaControls::Private {
  HWND hwnd = nullptr;
  ComPtr<ISystemMediaTransportControls> controls;
  ComPtr<ISystemMediaTransportControlsDisplayUpdater> display_updater;
  ButtonPressedHandler* handler = nullptr;
  EventRegistrationToken button_pressed_token = {};
};

WindowsMediaControls::WindowsMediaControls(Application* app, QObject* parent)
    : QObject(parent), app_(app), d_(new Private) {
  HINSTANCE instance = GetModuleHandleW(nullptr);

  // SMTC needs a window handle.  This one is never shown.
  WNDCLASSEXW window_class = {};
  window_class.cbSize = sizeof(window_class);
  window_class.lpfnWndProc = DefWindowProcW;
  window_class.hInstance = instance;
  window_class.lpszClassName = kWindowClassName;
  RegisterClassExW(&window_class);

  d_->hwnd = CreateWindowExW(0, kWindowClassName, L"Clementine", WS_OVERLAPPED,
                             0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
  if (!d_->hwnd) {
    qLog(Error) << "Failed to create window for media controls"
                << GetLastError();
    return;
  }

  SetWindowAppUserModelId(d_->hwnd, kAppUserModelId);

  ComPtr<ISystemMediaTransportControlsInterop> interop;
  HRESULT hr = PutString(
      QString::fromWCharArray(
          RuntimeClass_Windows_Media_SystemMediaTransportControls),
      [&interop](HSTRING class_id) {
        return RoGetActivationFactory(class_id, IID_PPV_ARGS(&interop));
      });
  if (FAILED(hr)) {
    qLog(Error) << "Failed to get SMTC activation factory" << hr;
    return;
  }

  hr = interop->GetForWindow(d_->hwnd, IID_PPV_ARGS(&d_->controls));
  if (FAILED(hr)) {
    qLog(Error) << "Failed to get SMTC for window" << hr;
    return;
  }

  d_->controls->put_IsEnabled(true);
  d_->controls->put_IsPlayEnabled(true);
  d_->controls->put_IsPauseEnabled(true);
  d_->controls->put_IsStopEnabled(true);
  d_->controls->put_IsNextEnabled(true);
  d_->controls->put_IsPreviousEnabled(true);

  hr = d_->controls->get_DisplayUpdater(&d_->display_updater);
  if (FAILED(hr)) {
    qLog(Warning) << "Failed to get SMTC display updater" << hr;
  }

  d_->handler = new ButtonPressedHandler(this);
  hr = d_->controls->add_ButtonPressed(d_->handler, &d_->button_pressed_token);
  if (FAILED(hr)) {
    qLog(Error) << "Failed to register SMTC button handler" << hr;
  }

  connect(app_->player()->engine(), SIGNAL(StateChanged(Engine::State)),
          SLOT(EngineStateChanged(Engine::State)));
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(CurrentSongChanged(Song)));
  connect(app_->current_art_loader(), SIGNAL(ArtLoaded(Song, QString, QImage)),
          SLOT(ArtLoaded(Song, QString, QImage)));

  EngineStateChanged(app_->player()->GetState());
}

WindowsMediaControls::~WindowsMediaControls() {
  if (d_->controls) {
    d_->controls->remove_ButtonPressed(d_->button_pressed_token);
    d_->controls->put_IsEnabled(false);
  }
  if (d_->handler) {
    d_->handler->Detach();
    d_->handler->Release();
  }
  d_->display_updater.Reset();
  d_->controls.Reset();
  if (d_->hwnd) {
    // The shell requires a window's AppUserModelID to be cleared before the
    // window is destroyed.
    SetWindowAppUserModelId(d_->hwnd, nullptr);
    DestroyWindow(d_->hwnd);
  }
}

void WindowsMediaControls::ButtonPressed(int button) {
  Player* player = app_->player();
  const Engine::State state = player->GetState();

  switch (button) {
    case ABI::Windows::Media::SystemMediaTransportControlsButton_Play:
      // Player::Play() restarts the track if it's already playing, which isn't
      // what a headphone play button should do.
      if (state != Engine::Playing) player->PlayPause();
      break;
    case ABI::Windows::Media::SystemMediaTransportControlsButton_Pause:
      // PlayPause() rather than Pause() so streams that can't be paused are
      // stopped instead.
      if (state == Engine::Playing) player->PlayPause();
      break;
    case ABI::Windows::Media::SystemMediaTransportControlsButton_Stop:
      player->Stop();
      break;
    case ABI::Windows::Media::SystemMediaTransportControlsButton_Next:
      player->Next();
      break;
    case ABI::Windows::Media::SystemMediaTransportControlsButton_Previous:
      player->Previous();
      break;
    default:
      break;
  }
}

void WindowsMediaControls::EngineStateChanged(Engine::State state) {
  if (!d_->controls) return;

  ABI::Windows::Media::MediaPlaybackStatus status;
  switch (state) {
    case Engine::Playing:
      status = ABI::Windows::Media::MediaPlaybackStatus_Playing;
      break;
    case Engine::Paused:
      status = ABI::Windows::Media::MediaPlaybackStatus_Paused;
      break;
    case Engine::Empty:
    case Engine::Idle:
    case Engine::Error:
    default:
      status = ABI::Windows::Media::MediaPlaybackStatus_Stopped;
      break;
  }
  d_->controls->put_PlaybackStatus(status);
}

// Send the metadata as soon as the song changes...
void WindowsMediaControls::CurrentSongChanged(const Song& song) {
  UpdateMetadata(song, QImage());
}

// ... and again with the cover once it's been loaded.  CurrentArtLoader only
// emits this for the current song.
void WindowsMediaControls::ArtLoaded(const Song& song, const QString& uri,
                                     const QImage& image) {
  // An empty URI means the song has no cover and image is a placeholder.
  UpdateMetadata(song, uri.isEmpty() ? QImage() : image);
}

void WindowsMediaControls::UpdateMetadata(const Song& song,
                                          const QImage& image) {
  ISystemMediaTransportControlsDisplayUpdater* updater =
      d_->display_updater.Get();
  if (!updater) return;

  updater->put_Type(ABI::Windows::Media::MediaPlaybackType_Music);

  ComPtr<IMusicDisplayProperties> properties;
  if (SUCCEEDED(updater->get_MusicProperties(&properties))) {
    PutString(song.PrettyTitle(),
              [&properties](HSTRING s) { return properties->put_Title(s); });
    PutString(song.artist(),
              [&properties](HSTRING s) { return properties->put_Artist(s); });
    PutString(song.effective_albumartist(), [&properties](HSTRING s) {
      return properties->put_AlbumArtist(s);
    });

    ComPtr<IMusicDisplayProperties2> properties2;
    if (SUCCEEDED(properties.As(&properties2))) {
      PutString(song.album(), [&properties2](HSTRING s) {
        return properties2->put_AlbumTitle(s);
      });
    }
  }

  // Encode the cover as a PNG in memory and wrap it in the WinRT stream types
  // SMTC wants.  This is all synchronous, unlike going through DataWriter.
  ComPtr<IRandomAccessStreamReference> thumbnail;
  if (!image.isNull()) {
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image
        .scaled(kThumbnailSize, kThumbnailSize, Qt::KeepAspectRatio,
                Qt::SmoothTransformation)
        .save(&buffer, "PNG");

    ComPtr<IStream> stream;
    stream.Attach(SHCreateMemStream(reinterpret_cast<const BYTE*>(data.data()),
                                    data.size()));

    ComPtr<IRandomAccessStream> random_access_stream;
    ComPtr<IRandomAccessStreamReferenceStatics> statics;
    HRESULT hr = stream ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr)) {
      hr = CreateRandomAccessStreamOverStream(
          stream.Get(), BSOS_DEFAULT, IID_PPV_ARGS(&random_access_stream));
    }
    if (SUCCEEDED(hr)) {
      hr = PutString(
          QString::fromWCharArray(
              RuntimeClass_Windows_Storage_Streams_RandomAccessStreamReference),
          [&statics](HSTRING class_id) {
            return RoGetActivationFactory(class_id, IID_PPV_ARGS(&statics));
          });
    }
    if (SUCCEEDED(hr)) {
      hr = statics->CreateFromStream(random_access_stream.Get(), &thumbnail);
    }
    if (FAILED(hr)) {
      qLog(Warning) << "Failed to create SMTC thumbnail" << hr;
    }
  }

  // A null thumbnail clears the previous song's cover.
  updater->put_Thumbnail(thumbnail.Get());
  updater->Update();
}
