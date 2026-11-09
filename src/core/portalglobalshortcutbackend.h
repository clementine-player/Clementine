/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef CORE_PORTALGLOBALSHORTCUTBACKEND_H_
#define CORE_PORTALGLOBALSHORTCUTBACKEND_H_

#include <QDBusConnection>
#include <QKeySequence>
#include <QString>
#include <QVariantMap>
#include <functional>

#include "globalshortcutbackend.h"

class QDBusObjectPath;

// Waits for the Response of one portal Request, calls handler with it, and
// deletes itself.
class PortalRequest : public QObject {
  Q_OBJECT

 public:
  using Handler = std::function<void(uint response, const QVariantMap&)>;

  PortalRequest(const QDBusConnection& bus, const QString& path,
                Handler handler, QObject* parent);
  ~PortalRequest();

  // For a call that failed, so there will be no Response.
  void Cancel();

 private slots:
  void Response(uint response, const QVariantMap& results);

 private:
  QDBusConnection bus_;
  QString path_;
  Handler handler_;
};

// Global shortcuts through the desktop portal's GlobalShortcuts interface,
// which works on Wayland, where nothing else can see keys pressed in other
// applications' windows. Clementine asks the desktop for its shortcuts, with
// the keys it would like for them, and the desktop decides the keys (often
// asking the user first) and says when one is pressed.
//
// https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html
class PortalGlobalShortcutBackend : public GlobalShortcutBackend {
  Q_OBJECT

 public:
  // Clementine's application ID, the name of its desktop file.
  static const char* kAppId;

  explicit PortalGlobalShortcutBackend(
      GlobalShortcuts* parent,
      const QDBusConnection& bus = QDBusConnection::sessionBus());

  // Whether the desktop offers global shortcuts through the portal.
  bool IsAvailable();
  // Whether the portal can open the desktop's settings for our shortcuts.
  bool CanConfigure();
  void Configure();

  // The key the portal is asked to prefer for a shortcut, in the format of
  // the XDG shortcuts specification ("CTRL+ALT+p"), or an empty string if
  // there's no key or it isn't one to ask for.
  static QString PreferredTrigger(const QKeySequence& key);

 protected:
  bool DoRegister() override;
  void DoUnregister() override;

 private slots:
  void Activated(const QDBusObjectPath& session, const QString& id,
                 qulonglong timestamp, const QVariantMap& options);

 private:
  using ResponseHandler = PortalRequest::Handler;

  int Version();
  void RegisterWithHostRegistry();
  QString NewToken();
  // Calls a portal method that answers with a Request, and handles the
  // Request's Response. options gets the handle_token the Request is named
  // with.
  void CallWithResponse(const QString& method, QList<QVariant> arguments,
                        QVariantMap options, ResponseHandler handler);
  void CreateSession(int generation);
  void BindShortcuts(int generation);
  void CloseSession();

  QDBusConnection bus_;
  // The portal's GlobalShortcuts version, 0 if there's no such portal, or -1
  // before asking.
  int version_;
  int next_token_;
  // Counts registrations, so the answer to an old one is ignored.
  int generation_;
  QString session_;
};

#endif  // CORE_PORTALGLOBALSHORTCUTBACKEND_H_
