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

 public slots:
  // Also called for a call that failed, which has no Response of its own.
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
// Desktops such as GNOME show a dialog when shortcuts they haven't seen are
// bound, so Clementine binds them only once the user has asked to set them
// up from the settings, and after that whenever it starts, when the desktop
// already knows them.
//
// https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html
class PortalGlobalShortcutBackend : public GlobalShortcutBackend {
  Q_OBJECT

 public:
  // Clementine's application ID, the name of its desktop file.
  static const char* kAppId;
  // Whether the user has set up the shortcuts with the desktop, in
  // GlobalShortcuts' settings group.
  static const char* kSetUpKey;

  explicit PortalGlobalShortcutBackend(
      GlobalShortcuts* parent,
      const QDBusConnection& bus = QDBusConnection::sessionBus());

  // Whether the desktop offers global shortcuts through the portal.
  bool IsAvailable();
  // Whether the user has set up the shortcuts, so they're bound.
  bool IsSetUp() const;
  // Binds the shortcuts, so the desktop may ask the user about them, and
  // remembers that they're set up if it works. SetUpFinished says how it
  // went.
  void SetUp();
  // Whether the portal can open the desktop's settings for our shortcuts.
  bool CanConfigure();
  void Configure();

  // The key the portal is asked to prefer for a shortcut, in the format of
  // the XDG shortcuts specification ("CTRL+ALT+p"), or an empty string if
  // there's no key or it isn't one to ask for.
  static QString PreferredTrigger(const QKeySequence& key);

 signals:
  void SetUpFinished(bool success);

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
  // done is called with whether the desktop bound them.
  void BindShortcuts(int generation, std::function<void(bool)> done);
  void CloseSession();

  QDBusConnection bus_;
  // The portal's GlobalShortcuts version, 0 if there's no such portal, or -1
  // before asking.
  int version_;
  int next_token_;
  // Counts registrations, so the answer to an old one is ignored.
  int generation_;
  QString session_;
  // Whether shortcuts were bound in this session: the portal binds them only
  // once per session.
  bool bound_;
};

#endif  // CORE_PORTALGLOBALSHORTCUTBACKEND_H_
