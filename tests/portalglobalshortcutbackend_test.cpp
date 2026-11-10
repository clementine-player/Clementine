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
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QDBusVirtualObject>
#include <QElapsedTimer>
#include <QProcess>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QThread>
#include <atomic>
#include <functional>
#include <memory>

#include "core/globalshortcuts.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

const char* kPortalService = "org.freedesktop.portal.Desktop";
const char* kPortalPath = "/org/freedesktop/portal/desktop";
const char* kPortalInterface = "org.freedesktop.portal.GlobalShortcuts";

// The portal's shortcuts, as a(sa{sv}).
struct FakeShortcut {
  QString id;
  QVariantMap properties;
};

QList<FakeShortcut> ReadShortcuts(const QDBusArgument& argument) {
  QList<FakeShortcut> shortcuts;
  argument.beginArray();
  while (!argument.atEnd()) {
    FakeShortcut shortcut;
    argument.beginStructure();
    argument >> shortcut.id >> shortcut.properties;
    argument.endStructure();
    shortcuts << shortcut;
  }
  argument.endArray();
  return shortcuts;
}

bool WaitFor(const std::function<bool()>& condition) {
  QElapsedTimer timer;
  timer.start();
  while (!condition()) {
    if (timer.elapsed() > 5000) return false;
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }
  return true;
}

// A desktop portal with GlobalShortcuts, which binds whatever it's asked to,
// unless it's told to answer as if the user had said no.
// It runs in a thread of its own, as the real one is in a process of its own,
// because the backend blocks on its first call.
class FakePortal : public QDBusVirtualObject {
 public:
  explicit FakePortal(const QDBusConnection& bus, int version)
      : bus_(bus), version_(version) {}

  QString introspect(const QString&) const override { return QString(); }

  bool handleMessage(const QDBusMessage& message,
                     const QDBusConnection& connection) override {
    const QString member = message.member();
    const QString interface = message.interface();
    const QList<QVariant> args = message.arguments();

    if (interface == "org.freedesktop.host.portal.Registry" &&
        member == "Register") {
      registered_app_id_ = args[0].toString();
      connection.send(message.createReply());
    } else if (interface == "org.freedesktop.DBus.Properties" &&
               member == "Get") {
      if (version_ == 0) {
        connection.send(message.createErrorReply(
            "org.freedesktop.DBus.Error.UnknownInterface", "No such portal"));
      } else {
        connection.send(
            message.createReply(QVariant::fromValue(QDBusVariant(version_))));
      }
    } else if (interface == kPortalInterface && member == "CreateSession") {
      const QVariantMap options = qdbus_cast<QVariantMap>(args[0]);
      const QString session =
          QString("%1/session/%2/%3")
              .arg(kPortalPath, Sender(message),
                   options["session_handle_token"].toString());
      sessions_ << session;
      QVariantMap results;
      results["session_handle"] = session;
      Respond(message, options["handle_token"].toString(), results);
    } else if (interface == kPortalInterface && member == "BindShortcuts") {
      const QVariantMap options = qdbus_cast<QVariantMap>(args[3]);
      ++binds_;
      if (refuse_binds_) {
        Respond(message, options["handle_token"].toString(), QVariantMap(), 1);
        return true;
      }
      bound_session_ = qdbus_cast<QDBusObjectPath>(args[0]).path();
      bound_ = ReadShortcuts(args[1].value<QDBusArgument>());
      Respond(message, options["handle_token"].toString(), QVariantMap());
    } else if (interface == kPortalInterface &&
               member == "ConfigureShortcuts") {
      configured_ = true;
      connection.send(message.createReply());
    } else if (interface == "org.freedesktop.portal.Session" &&
               member == "Close") {
      closed_ << message.path();
      connection.send(message.createReply());
    } else {
      return false;
    }
    return true;
  }

  void Activate(const QString& session, const QString& id) {
    QDBusMessage signal =
        QDBusMessage::createSignal(kPortalPath, kPortalInterface, "Activated");
    signal << QVariant::fromValue(QDBusObjectPath(session)) << id
           << qulonglong(0) << QVariantMap();
    bus_.send(signal);
  }

  QString registered_app_id_;
  QStringList sessions_;
  QString bound_session_;
  QList<FakeShortcut> bound_;
  QStringList closed_;
  std::atomic<bool> configured_ = false;
  std::atomic<int> binds_ = 0;
  // Answers BindShortcuts as if the user had cancelled the desktop's dialog.
  std::atomic<bool> refuse_binds_ = false;

 private:
  static QString Sender(const QDBusMessage& message) {
    return message.service().mid(1).replace('.', '_');
  }

  // Answers a call with its Request's path, then sends the Request's
  // Response.
  void Respond(const QDBusMessage& message, const QString& token,
               const QVariantMap& results, uint code = 0) {
    const QString request =
        QString("%1/request/%2/%3").arg(kPortalPath, Sender(message), token);
    bus_.send(
        message.createReply(QVariant::fromValue(QDBusObjectPath(request))));
    QDBusMessage response = QDBusMessage::createSignal(
        request, "org.freedesktop.portal.Request", "Response");
    response << code << results;
    bus_.send(response);
  }

  QDBusConnection bus_;
  int version_;
};

// A bus of the test's own, as the session bus, so GlobalShortcuts and the
// fake portal meet on it.
class PortalGlobalShortcutBackendTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    settings_dir_ = new QTemporaryDir;
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       settings_dir_->path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       settings_dir_->path());

    const QString daemon = QStandardPaths::findExecutable("dbus-daemon");
    if (daemon.isEmpty()) return;

    daemon_ = new QProcess;
    daemon_->start(daemon, {"--session", "--nofork", "--print-address=1"});
    if (!daemon_->waitForReadyRead(10000)) return;
    address_ = QString::fromUtf8(daemon_->readLine()).trimmed();
    qputenv("DBUS_SESSION_BUS_ADDRESS", address_.toUtf8());
  }

  static void TearDownTestSuite() {
    delete settings_dir_;
    settings_dir_ = nullptr;
    if (daemon_) {
      daemon_->kill();
      daemon_->waitForFinished();
      delete daemon_;
      daemon_ = nullptr;
    }
  }

  void SetUp() override {
    if (address_.isEmpty()) GTEST_SKIP() << "No dbus-daemon to run a bus with";
    QSettings().remove(GlobalShortcuts::kSettingsGroup);
    manager_ = std::make_unique<GlobalShortcuts>();
  }

  void StartPortal(int version) {
    portal_bus_ =
        std::make_unique<QDBusConnection>(QDBusConnection::connectToBus(
            address_, QString("portal%1").arg(next_connection_++)));
    ASSERT_TRUE(portal_bus_->isConnected());
    portal_ = std::make_unique<FakePortal>(*portal_bus_, version);
    portal_thread_ = std::make_unique<QThread>();
    portal_->moveToThread(portal_thread_.get());
    portal_thread_->start();
    ASSERT_TRUE(portal_bus_->registerVirtualObject(kPortalPath, portal_.get(),
                                                   QDBusConnection::SubPath));
    ASSERT_TRUE(portal_bus_->registerService(kPortalService));
  }

  void TearDown() override {
    manager_.reset();
    if (portal_bus_) {
      portal_bus_->unregisterService(kPortalService);
      portal_bus_->unregisterObject(kPortalPath,
                                    QDBusConnection::UnregisterTree);
      QDBusConnection::disconnectFromBus(portal_bus_->name());
    }
    if (portal_thread_) {
      portal_thread_->quit();
      portal_thread_->wait();
    }
  }

  // A backend on a connection of its own, as GlobalShortcuts' would be.
  std::unique_ptr<PortalGlobalShortcutBackend> NewBackend() {
    return std::make_unique<PortalGlobalShortcutBackend>(
        manager_.get(),
        QDBusConnection::connectToBus(
            address_, QString("backend%1").arg(next_connection_++)));
  }

  // As if the user had set up the shortcuts when Clementine last ran.
  static void SetSetUp() {
    QSettings s;
    s.beginGroup(GlobalShortcuts::kSettingsGroup);
    s.setValue(PortalGlobalShortcutBackend::kSetUpKey, true);
  }

  // Registers, and waits for the portal's answers.
  static void RegisterAndWait(PortalGlobalShortcutBackend* backend) {
    QSignalSpy finished(backend, &GlobalShortcutBackend::RegisterFinished);
    ASSERT_TRUE(backend->Register());
    ASSERT_TRUE(finished.wait(5000));
    EXPECT_TRUE(finished[0][0].toBool());
  }

  // Sets up, and returns whether it worked.
  static bool SetUpAndWait(PortalGlobalShortcutBackend* backend) {
    QSignalSpy finished(backend, &PortalGlobalShortcutBackend::SetUpFinished);
    backend->SetUp();
    if (finished.isEmpty() && !finished.wait(5000)) return false;
    return finished[0][0].toBool();
  }

  static QTemporaryDir* settings_dir_;
  static QProcess* daemon_;
  static QString address_;
  static int next_connection_;

  std::unique_ptr<GlobalShortcuts> manager_;
  std::unique_ptr<QDBusConnection> portal_bus_;
  std::unique_ptr<FakePortal> portal_;
  std::unique_ptr<QThread> portal_thread_;
};

QTemporaryDir* PortalGlobalShortcutBackendTest::settings_dir_ = nullptr;
QProcess* PortalGlobalShortcutBackendTest::daemon_ = nullptr;
QString PortalGlobalShortcutBackendTest::address_;
int PortalGlobalShortcutBackendTest::next_connection_ = 0;

TEST(PortalPreferredTriggerTest, ModifiersAndKey) {
  EXPECT_EQ("CTRL+ALT+p", PortalGlobalShortcutBackend::PreferredTrigger(
                              QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_P)));
  EXPECT_EQ("SHIFT+LOGO+F5",
            PortalGlobalShortcutBackend::PreferredTrigger(
                QKeySequence(Qt::SHIFT | Qt::META | Qt::Key_F5)));
  EXPECT_EQ("CTRL+7", PortalGlobalShortcutBackend::PreferredTrigger(
                          QKeySequence(Qt::CTRL | Qt::Key_7)));
  EXPECT_EQ("ALT+Prior", PortalGlobalShortcutBackend::PreferredTrigger(
                             QKeySequence(Qt::ALT | Qt::Key_PageUp)));
  EXPECT_EQ("CTRL+space", PortalGlobalShortcutBackend::PreferredTrigger(
                              QKeySequence(Qt::CTRL | Qt::Key_Space)));
}

TEST(PortalPreferredTriggerTest, NoneForMediaKeysOrNoKey) {
  EXPECT_EQ("", PortalGlobalShortcutBackend::PreferredTrigger(QKeySequence()));
  EXPECT_EQ("", PortalGlobalShortcutBackend::PreferredTrigger(
                    QKeySequence(Qt::Key_MediaPlay)));
  EXPECT_EQ("", PortalGlobalShortcutBackend::PreferredTrigger(
                    QKeySequence(Qt::SHIFT | Qt::Key_MediaNext)));
  EXPECT_EQ("", PortalGlobalShortcutBackend::PreferredTrigger(
                    QKeySequence(Qt::Key_VolumeUp)));
}

TEST_F(PortalGlobalShortcutBackendTest, UnavailableWithoutPortal) {
  StartPortal(0);
  std::unique_ptr<PortalGlobalShortcutBackend> backend = NewBackend();
  EXPECT_FALSE(backend->IsAvailable());
  EXPECT_FALSE(backend->Register());
  EXPECT_FALSE(backend->is_active());
}

TEST_F(PortalGlobalShortcutBackendTest, BindsShortcutsOnceSetUp) {
  StartPortal(1);
  SetSetUp();
  manager_->shortcuts()["show_osd"].action->setShortcut(
      QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_O));

  std::unique_ptr<PortalGlobalShortcutBackend> backend = NewBackend();
  QSignalSpy finished(backend.get(), &GlobalShortcutBackend::RegisterFinished);
  ASSERT_TRUE(backend->Register());
  ASSERT_TRUE(finished.wait(5000));
  EXPECT_TRUE(finished[0][0].toBool());

  EXPECT_EQ(PortalGlobalShortcutBackend::kAppId, portal_->registered_app_id_);
  ASSERT_EQ(1, portal_->sessions_.size());
  EXPECT_EQ(portal_->sessions_[0], portal_->bound_session_);

  // Every shortcut is bound, so the desktop can give any of them a key, but
  // only keys that aren't media keys are asked for.
  EXPECT_EQ(manager_->shortcuts().size(), portal_->bound_.size());
  QMap<QString, QVariantMap> bound;
  for (const FakeShortcut& shortcut : portal_->bound_) {
    bound[shortcut.id] = shortcut.properties;
  }
  EXPECT_EQ("Play/Pause", bound["play_pause"]["description"].toString());
  EXPECT_FALSE(bound["play_pause"].contains("preferred_trigger"));
  EXPECT_EQ("CTRL+ALT+o", bound["show_osd"]["preferred_trigger"].toString());
  EXPECT_FALSE(bound["stop_after"].contains("preferred_trigger"));

  QSignalSpy stop(manager_.get(), &GlobalShortcuts::Stop);
  portal_->Activate(portal_->sessions_[0], "stop");
  ASSERT_TRUE(stop.wait(5000));

  // Another session's shortcut isn't ours.
  QSignalSpy next(manager_.get(), &GlobalShortcuts::Next);
  portal_->Activate("/org/freedesktop/portal/desktop/session/x/y",
                    "next_track");
  EXPECT_FALSE(next.wait(500));
}

TEST_F(PortalGlobalShortcutBackendTest, DoesntBindUntilSetUp) {
  StartPortal(1);
  std::unique_ptr<PortalGlobalShortcutBackend> backend = NewBackend();
  RegisterAndWait(backend.get());

  // Binding could show the desktop's dialog, which shouldn't appear just
  // because Clementine started.
  ASSERT_EQ(1, portal_->sessions_.size());
  EXPECT_EQ(0, portal_->binds_.load());
  EXPECT_TRUE(backend->is_active());
  EXPECT_FALSE(backend->IsSetUp());

  ASSERT_TRUE(SetUpAndWait(backend.get()));
  EXPECT_EQ(portal_->sessions_[0], portal_->bound_session_);
  EXPECT_EQ(manager_->shortcuts().size(), portal_->bound_.size());
  EXPECT_TRUE(backend->IsSetUp());

  QSignalSpy stop(manager_.get(), &GlobalShortcuts::Stop);
  portal_->Activate(portal_->sessions_[0], "stop");
  ASSERT_TRUE(stop.wait(5000));

  // Next time, they're bound when Clementine starts.
  std::unique_ptr<PortalGlobalShortcutBackend> next = NewBackend();
  RegisterAndWait(next.get());
  ASSERT_EQ(2, portal_->sessions_.size());
  EXPECT_EQ(2, portal_->binds_.load());
  EXPECT_EQ(portal_->sessions_[1], portal_->bound_session_);
}

TEST_F(PortalGlobalShortcutBackendTest, CancelledSetUpCanBeTriedAgain) {
  StartPortal(1);
  std::unique_ptr<PortalGlobalShortcutBackend> backend = NewBackend();
  RegisterAndWait(backend.get());
  ASSERT_EQ(1, portal_->sessions_.size());

  portal_->refuse_binds_ = true;
  QSignalSpy new_session(backend.get(),
                         &GlobalShortcutBackend::RegisterFinished);
  EXPECT_FALSE(SetUpAndWait(backend.get()));
  EXPECT_FALSE(backend->IsSetUp());

  // The portal binds once per session, so trying again takes another one.
  ASSERT_TRUE(new_session.wait(5000));
  ASSERT_EQ(2, portal_->sessions_.size());
  ASSERT_EQ(1, portal_->closed_.size());
  EXPECT_EQ(portal_->sessions_[0], portal_->closed_[0]);

  portal_->refuse_binds_ = false;
  ASSERT_TRUE(SetUpAndWait(backend.get()));
  EXPECT_EQ(portal_->sessions_[1], portal_->bound_session_);
  EXPECT_TRUE(backend->IsSetUp());
}

TEST_F(PortalGlobalShortcutBackendTest, UnregisterClosesTheSession) {
  StartPortal(1);
  std::unique_ptr<PortalGlobalShortcutBackend> backend = NewBackend();
  QSignalSpy finished(backend.get(), &GlobalShortcutBackend::RegisterFinished);
  ASSERT_TRUE(backend->Register());
  ASSERT_TRUE(finished.wait(5000));
  ASSERT_EQ(1, portal_->sessions_.size());

  backend->Unregister();
  EXPECT_FALSE(backend->is_active());

  QSignalSpy stop(manager_.get(), &GlobalShortcuts::Stop);
  portal_->Activate(portal_->sessions_[0], "stop");
  EXPECT_FALSE(stop.wait(500));
  ASSERT_EQ(1, portal_->closed_.size());
  EXPECT_EQ(portal_->sessions_[0], portal_->closed_[0]);
}

TEST(PortalGnomeSettingsCommandTest, OnlyOnGnome) {
  EXPECT_EQ(QStringList() << "/usr/bin/gnome-control-center"
                          << "applications"
                          << PortalGlobalShortcutBackend::kAppId,
            PortalGlobalShortcutBackend::GnomeSettingsCommand(
                "GNOME", "/usr/bin/gnome-control-center"));
  // Ubuntu's session is "ubuntu:GNOME".
  EXPECT_EQ(3, PortalGlobalShortcutBackend::GnomeSettingsCommand(
                   "ubuntu:GNOME", "/usr/bin/gnome-control-center")
                   .size());
  EXPECT_TRUE(PortalGlobalShortcutBackend::GnomeSettingsCommand(
                  "Hyprland", "/usr/bin/gnome-control-center")
                  .isEmpty());
  EXPECT_TRUE(
      PortalGlobalShortcutBackend::GnomeSettingsCommand("GNOME", QString())
          .isEmpty());
}

TEST_F(PortalGlobalShortcutBackendTest, CantConfigureBeforeVersion2) {
  StartPortal(1);
  SetSetUp();
  // Not GNOME, so there's no Settings page to open instead.
  qputenv("XDG_CURRENT_DESKTOP", "Hyprland");
  std::unique_ptr<PortalGlobalShortcutBackend> backend = NewBackend();
  QSignalSpy finished(backend.get(), &GlobalShortcutBackend::RegisterFinished);
  ASSERT_TRUE(backend->Register());
  ASSERT_TRUE(finished.wait(5000));
  EXPECT_FALSE(backend->CanConfigure());
}

TEST_F(PortalGlobalShortcutBackendTest, ConfiguresFromVersion2) {
  StartPortal(2);
  std::unique_ptr<PortalGlobalShortcutBackend> backend = NewBackend();
  RegisterAndWait(backend.get());
  // There's nothing to configure until the shortcuts are bound.
  EXPECT_FALSE(backend->CanConfigure());
  ASSERT_TRUE(SetUpAndWait(backend.get()));
  ASSERT_TRUE(backend->CanConfigure());
  backend->Configure();
  EXPECT_TRUE(WaitFor([this]() { return portal_->configured_.load(); }));
}

}  // namespace
