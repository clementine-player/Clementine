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

#include "core/portalglobalshortcutbackend.h"

#include <QAction>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QMap>

#include "core/globalshortcuts.h"
#include "core/logging.h"

// One shortcut as the portal sends and receives them: an ID, and properties
// such as its description.
struct PortalShortcut {
  QString id;
  QVariantMap properties;
};
Q_DECLARE_METATYPE(PortalShortcut)

// QDBusArgument's operators take it by non-const reference.
QDBusArgument& operator<<(QDBusArgument& argument,
                          const PortalShortcut& shortcut) {
  argument.beginStructure();
  argument << shortcut.id << shortcut.properties;
  argument.endStructure();
  return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument,
                                PortalShortcut& shortcut) {
  argument.beginStructure();
  argument >> shortcut.id >> shortcut.properties;
  argument.endStructure();
  return argument;
}

namespace {

const char* kPortalService = "org.freedesktop.portal.Desktop";
const char* kPortalPath = "/org/freedesktop/portal/desktop";
const char* kPortalInterface = "org.freedesktop.portal.GlobalShortcuts";
const char* kRequestInterface = "org.freedesktop.portal.Request";
const char* kSessionInterface = "org.freedesktop.portal.Session";
const char* kRegistryInterface = "org.freedesktop.host.portal.Registry";

// The portal's first answer can wait for it to start.
const int kVersionTimeoutMsec = 5000;

// The portal names Requests and Sessions after the caller's unique name on
// the bus, without its colon and with underscores for its dots, and the token
// the caller gave.
QString PortalObjectPath(const QDBusConnection& bus, const QString& kind,
                         const QString& token) {
  QString sender = bus.baseService();
  sender.remove(0, 1).replace('.', '_');
  return QString("%1/%2/%3/%4").arg(kPortalPath, kind, sender, token);
}

// An object path the portal sends, which it types as a string in some
// results and an object path in others.
QString ObjectPath(const QVariant& value) {
  if (value.canConvert<QDBusObjectPath>()) {
    return value.value<QDBusObjectPath>().path();
  }
  return value.toString();
}

// The XKB name of a key, as the XDG shortcuts specification wants it.
QString KeysymName(Qt::Key key) {
  if (key >= Qt::Key_A && key <= Qt::Key_Z) {
    return QString(QChar('a' + (key - Qt::Key_A)));
  }
  if (key >= Qt::Key_0 && key <= Qt::Key_9) {
    return QString(QChar('0' + (key - Qt::Key_0)));
  }
  if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
    return QString("F%1").arg(key - Qt::Key_F1 + 1);
  }

  static const QMap<Qt::Key, QString> kNames = {
      {Qt::Key_Space, "space"},
      {Qt::Key_Return, "Return"},
      {Qt::Key_Enter, "KP_Enter"},
      {Qt::Key_Escape, "Escape"},
      {Qt::Key_Tab, "Tab"},
      {Qt::Key_Backspace, "BackSpace"},
      {Qt::Key_Insert, "Insert"},
      {Qt::Key_Delete, "Delete"},
      {Qt::Key_Home, "Home"},
      {Qt::Key_End, "End"},
      {Qt::Key_PageUp, "Prior"},
      {Qt::Key_PageDown, "Next"},
      {Qt::Key_Left, "Left"},
      {Qt::Key_Right, "Right"},
      {Qt::Key_Up, "Up"},
      {Qt::Key_Down, "Down"},
      {Qt::Key_Pause, "Pause"},
      {Qt::Key_Print, "Print"},
      {Qt::Key_Comma, "comma"},
      {Qt::Key_Period, "period"},
      {Qt::Key_Slash, "slash"},
      {Qt::Key_Backslash, "backslash"},
      {Qt::Key_Minus, "minus"},
      {Qt::Key_Equal, "equal"},
      {Qt::Key_Semicolon, "semicolon"},
      {Qt::Key_Apostrophe, "apostrophe"},
      {Qt::Key_BracketLeft, "bracketleft"},
      {Qt::Key_BracketRight, "bracketright"},
      {Qt::Key_QuoteLeft, "grave"},
  };
  return kNames.value(key);
}

}  // namespace

PortalRequest::PortalRequest(const QDBusConnection& bus, const QString& path,
                             Handler handler, QObject* parent)
    : QObject(parent), bus_(bus), path_(path), handler_(std::move(handler)) {
  bus_.connect(kPortalService, path_, kRequestInterface, "Response", this,
               SLOT(Response(uint, QVariantMap)));
}

PortalRequest::~PortalRequest() {
  bus_.disconnect(kPortalService, path_, kRequestInterface, "Response", this,
                  SLOT(Response(uint, QVariantMap)));
}

void PortalRequest::Cancel() { deleteLater(); }

void PortalRequest::Response(uint response, const QVariantMap& results) {
  // Only once, even if another Response arrives before this is deleted.
  if (handler_) {
    Handler handler = std::move(handler_);
    handler_ = nullptr;
    handler(response, results);
  }
  deleteLater();
}

const char* PortalGlobalShortcutBackend::kAppId =
    "org.clementine_player.Clementine";

PortalGlobalShortcutBackend::PortalGlobalShortcutBackend(
    GlobalShortcuts* parent, const QDBusConnection& bus)
    : GlobalShortcutBackend(parent),
      bus_(bus),
      version_(-1),
      next_token_(0),
      generation_(0) {
  qDBusRegisterMetaType<PortalShortcut>();
  qDBusRegisterMetaType<QList<PortalShortcut>>();
}

void PortalGlobalShortcutBackend::RegisterWithHostRegistry() {
  // An application that isn't in a sandbox tells the portal its ID, which
  // GNOME needs before it gives out shortcuts, before it calls any portal.
  // Older portals don't have the registry, and applications in a sandbox
  // already have an ID: either way, the error doesn't matter.
  QDBusMessage message = QDBusMessage::createMethodCall(
      kPortalService, kPortalPath, kRegistryInterface, "Register");
  message << QString(kAppId) << QVariantMap();
  bus_.call(message, QDBus::Block, kVersionTimeoutMsec);
}

int PortalGlobalShortcutBackend::Version() {
  if (version_ >= 0) return version_;

  version_ = 0;
  if (!bus_.isConnected()) return version_;

  RegisterWithHostRegistry();

  QDBusMessage message = QDBusMessage::createMethodCall(
      kPortalService, kPortalPath, "org.freedesktop.DBus.Properties", "Get");
  message << QString(kPortalInterface) << QString("version");
  QDBusMessage reply = bus_.call(message, QDBus::Block, kVersionTimeoutMsec);
  if (reply.type() != QDBusMessage::ReplyMessage ||
      reply.arguments().isEmpty()) {
    qLog(Debug) << "The desktop has no global shortcuts portal:"
                << reply.errorMessage();
    return version_;
  }

  version_ = reply.arguments()[0].value<QDBusVariant>().variant().toInt();
  qLog(Debug) << "Global shortcuts portal version" << version_;
  return version_;
}

bool PortalGlobalShortcutBackend::IsAvailable() { return Version() >= 1; }

// ConfigureShortcuts is new in version 2.
bool PortalGlobalShortcutBackend::CanConfigure() {
  return Version() >= 2 && !session_.isEmpty();
}

void PortalGlobalShortcutBackend::Configure() {
  if (!CanConfigure()) return;

  QDBusMessage message = QDBusMessage::createMethodCall(
      kPortalService, kPortalPath, kPortalInterface, "ConfigureShortcuts");
  message << QVariant::fromValue(QDBusObjectPath(session_)) << QString()
          << QVariantMap();
  bus_.asyncCall(message);
}

QString PortalGlobalShortcutBackend::PreferredTrigger(const QKeySequence& key) {
  if (key.isEmpty()) return QString();

  // Media and volume keys aren't asked for: on desktops with the portal they
  // already reach Clementine over MPRIS, so a shortcut on them would either
  // be refused or act twice.
  const QKeyCombination combination = key[0];
  const QString name = KeysymName(combination.key());
  if (name.isEmpty()) return QString();

  QStringList parts;
  const Qt::KeyboardModifiers modifiers = combination.keyboardModifiers();
  if (modifiers & Qt::ControlModifier) parts << "CTRL";
  if (modifiers & Qt::AltModifier) parts << "ALT";
  if (modifiers & Qt::ShiftModifier) parts << "SHIFT";
  if (modifiers & Qt::MetaModifier) parts << "LOGO";
  parts << name;
  return parts.join('+');
}

QString PortalGlobalShortcutBackend::NewToken() {
  return QString("clementine%1").arg(next_token_++);
}

void PortalGlobalShortcutBackend::CallWithResponse(const QString& method,
                                                   QList<QVariant> arguments,
                                                   QVariantMap options,
                                                   ResponseHandler handler) {
  // The Request is listened for before the call, so its Response can't come
  // first.
  const QString token = NewToken();
  options["handle_token"] = token;
  PortalRequest* request = new PortalRequest(
      bus_, PortalObjectPath(bus_, "request", token), std::move(handler), this);

  QDBusMessage message = QDBusMessage::createMethodCall(
      kPortalService, kPortalPath, kPortalInterface, method);
  arguments << options;
  message.setArguments(arguments);

  QDBusPendingCallWatcher* watcher =
      new QDBusPendingCallWatcher(bus_.asyncCall(message), this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this, method, request](QDBusPendingCallWatcher* watcher) {
            watcher->deleteLater();
            QDBusPendingReply<QDBusObjectPath> reply = *watcher;
            if (reply.isError()) {
              qLog(Warning) << "The global shortcuts portal's" << method
                            << "failed:" << reply.error().message();
              request->Cancel();
              emit RegisterFinished(false);
            }
          });
}

bool PortalGlobalShortcutBackend::DoRegister() {
  if (!IsAvailable()) return false;

  CreateSession(++generation_);
  return true;
}

void PortalGlobalShortcutBackend::CreateSession(int generation) {
  QVariantMap options;
  options["session_handle_token"] = NewToken();
  CallWithResponse(
      "CreateSession", {}, options,
      [this, generation](uint response, const QVariantMap& results) {
        const QString session = ObjectPath(results["session_handle"]);
        if (response != 0 || session.isEmpty()) {
          qLog(Warning)
              << "The global shortcuts portal didn't create a session";
          emit RegisterFinished(false);
          return;
        }
        if (generation != generation_) {
          // Unregistered since: this session isn't wanted any more.
          QDBusMessage message = QDBusMessage::createMethodCall(
              kPortalService, session, kSessionInterface, "Close");
          bus_.asyncCall(message);
          return;
        }

        session_ = session;
        bus_.connect(
            kPortalService, kPortalPath, kPortalInterface, "Activated", this,
            SLOT(Activated(QDBusObjectPath, QString, qulonglong, QVariantMap)));
        BindShortcuts(generation);
      });
}

void PortalGlobalShortcutBackend::BindShortcuts(int generation) {
  QList<PortalShortcut> shortcuts;
  for (const GlobalShortcuts::Shortcut& shortcut : manager_->shortcuts()) {
    PortalShortcut portal_shortcut;
    portal_shortcut.id = shortcut.id;
    portal_shortcut.properties["description"] =
        shortcut.action->text().remove('&');
    const QString trigger = PreferredTrigger(shortcut.action->shortcut());
    if (!trigger.isEmpty()) {
      portal_shortcut.properties["preferred_trigger"] = trigger;
    }
    shortcuts << portal_shortcut;
  }

  // No parent window: the settings dialog may not be open.
  CallWithResponse(
      "BindShortcuts",
      {QVariant::fromValue(QDBusObjectPath(session_)),
       QVariant::fromValue(shortcuts), QString()},
      QVariantMap(),
      [this, generation](uint response, const QVariantMap& results) {
        if (generation != generation_) return;
        if (response != 0) {
          // 1 is the user saying no.
          qLog(Warning) << "The global shortcuts portal didn't bind shortcuts:"
                        << response;
          emit RegisterFinished(false);
          return;
        }

        const QList<PortalShortcut> bound =
            qdbus_cast<QList<PortalShortcut>>(results["shortcuts"]);
        for (const PortalShortcut& shortcut : bound) {
          qLog(Debug) << "Global shortcut" << shortcut.id << "is"
                      << shortcut.properties["trigger_description"].toString();
        }
        emit RegisterFinished(true);
      });
}

void PortalGlobalShortcutBackend::DoUnregister() {
  ++generation_;
  CloseSession();
}

void PortalGlobalShortcutBackend::CloseSession() {
  if (session_.isEmpty()) return;

  bus_.disconnect(
      kPortalService, kPortalPath, kPortalInterface, "Activated", this,
      SLOT(Activated(QDBusObjectPath, QString, qulonglong, QVariantMap)));
  QDBusMessage message = QDBusMessage::createMethodCall(
      kPortalService, session_, kSessionInterface, "Close");
  bus_.asyncCall(message);
  session_.clear();
}

void PortalGlobalShortcutBackend::Activated(const QDBusObjectPath& session,
                                            const QString& id, qulonglong,
                                            const QVariantMap&) {
  if (session.path() != session_) return;

  const QMap<QString, GlobalShortcuts::Shortcut> shortcuts =
      manager_->shortcuts();
  auto it = shortcuts.find(id);
  if (it == shortcuts.end()) return;
  if (it->action->isEnabled()) it->action->trigger();
}
