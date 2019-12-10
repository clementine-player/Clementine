#include "core/logging.h"
#include "kglobalaccelglobalshortcutbackend.h"

#include <QAction>
#include <QGuiApplication>

#ifdef HAVE_DBUS

#include <dbus/kglobalaccel.h>
#include <dbus/kglobalaccelcomponent.h>

#endif

// Most of this file is based on the KGlobalAccel sources
// (https://phabricator.kde.org/source/kglobalaccel)

namespace {
#ifdef HAVE_DBUS
QString ComponentDisplayName() {
  return QGuiApplication::applicationDisplayName().isEmpty()
             ? QCoreApplication::applicationName()
             : QGuiApplication::applicationDisplayName();
}

QString ComponentUniqueName() { return QCoreApplication::applicationName(); }

const QString& IdActionUniqueName(const QStringList& id) { return id.at(1); }

bool IsCorrectMediaKeyShortcut(const GlobalShortcuts::Shortcut& shortcut) {
  if (shortcut.id == QStringLiteral("play_pause")) {
    return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaPlay);
  } else if (shortcut.id == QStringLiteral("stop")) {
    return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaStop);
  } else if (shortcut.id == QStringLiteral("next_track")) {
    return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaNext);
  } else if (shortcut.id == QStringLiteral("prev_track")) {
    return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaPrevious);
  } else {
    return false;
  }
}
#endif  // HAVE_DBUS
}  // namespace

#ifdef HAVE_DBUS

KGlobalAccelShortcutBackend::KGlobalAccelShortcutBackend(
    GlobalShortcuts* parent)
    : GlobalShortcutBackend(parent), iface_(nullptr), component_(nullptr) {}

#else   // HAVE_DBUS
KGlobalAccelShortcutBackend::KGlobalAccelShortcutBackend(
    GlobalShortcuts* parent)
    : GlobalShortcutBackend(parent) {}
#endif  // HAVE_DBUS

bool KGlobalAccelShortcutBackend::IsKGlobalAccelAvailable() {
#ifdef HAVE_DBUS
  return QDBusConnection::sessionBus().interface()->isServiceRegistered(
      kService);
#else   // HAVE_DBUS
  return false;
#endif  // HAVE_DBUS
}

bool KGlobalAccelShortcutBackend::DoRegister() {
#ifdef HAVE_DBUS
  qLog(Debug) << "Registering shortcuts";

  if (!AcquireInterface()) return false;

  bool complete = true;
  for (const GlobalShortcuts::Shortcut& shortcut :
       manager_->shortcuts().values()) {
    if (shortcut.action->shortcut().isEmpty()) continue;

    if (!RegisterShortcut(shortcut)) complete = false;
  }

  if (!AcquireComponent()) return false;

  QObject::connect(component_,
                   &OrgKdeKglobalaccelComponentInterface::globalShortcutPressed,
                   this, &KGlobalAccelShortcutBackend::OnShortcutPressed,
                   Qt::UniqueConnection);

  return complete;
#else   // HAVE_DBUS
  qLog(Warning) << "dbus not available";
  return false;
#endif  // HAVE_DBUS
}

void KGlobalAccelShortcutBackend::DoUnregister() {
#ifdef HAVE_DBUS
  if (!AcquireInterface()) return;

  if (!AcquireComponent()) return;

  for (const GlobalShortcuts::Shortcut& shortcut : manager_->shortcuts())
    UnregisterAction(shortcut.id, shortcut.action);

  QObject::disconnect(
      component_, &OrgKdeKglobalaccelComponentInterface::globalShortcutPressed,
      this, &KGlobalAccelShortcutBackend::OnShortcutPressed);
#endif  // HAVE_DBUS
}

#ifdef HAVE_DBUS

const char* KGlobalAccelShortcutBackend::kService = "org.kde.kglobalaccel";
const char* KGlobalAccelShortcutBackend::kPath = "/kglobalaccel";

bool KGlobalAccelShortcutBackend::AcquireComponent() {
  Q_ASSERT(iface_ && iface_->isValid());

  if (component_) return true;

  QDBusReply<QDBusObjectPath> reply =
      iface_->getComponent(ComponentUniqueName());
  if (!reply.isValid()) {
    if (reply.error().name() ==
        QLatin1String("org.kde.kglobalaccel.NoSuchComponent"))
      return false;

    qLog(Warning) << "Failed to get DBus path for KGlobalAccel component";
    return false;
  }

  component_ = new org::kde::kglobalaccel::Component(
      kService, reply.value().path(), QDBusConnection::sessionBus(), iface_);

  if (!component_->isValid()) {
    qLog(Warning) << "Failed to get KGlobalAccel component:"
                  << QDBusConnection::sessionBus().lastError();
    component_->deleteLater();
    component_ = nullptr;
    return false;
  }

  return true;
}

bool KGlobalAccelShortcutBackend::AcquireInterface() {
  if (iface_ && iface_->isValid()) return true;

  if (IsKGlobalAccelAvailable()) {
    iface_ = new OrgKdeKGlobalAccelInterface(
        kService, kPath, QDBusConnection::sessionBus(), this);
  }

  if (iface_ && iface_->isValid()) return true;

  if (!iface_)
    qLog(Warning) << "KGlobalAccel daemon not registered";
  else if (!iface_->isValid())
    qLog(Warning) << "KGlobalAccel daemon is not valid";
  return false;
}

QStringList KGlobalAccelShortcutBackend::GetId(const QString& name,
                                               const QAction* action) {
  Q_ASSERT(action);

  QStringList ret;
  ret << ComponentUniqueName();
  ret << name;
  ret << ComponentDisplayName();
  ret << action->text().replace(QLatin1Char('&'), QStringLiteral(""));
  if (ret.back().isEmpty()) ret.back() = name;
  return ret;
}

QList<int> KGlobalAccelShortcutBackend::ToIntList(
    const QList<QKeySequence>& seq) {
  QList<int> ret;
  for (const QKeySequence& sequence : seq) {
    ret.append(sequence[0]);
  }
  while (!ret.isEmpty() && ret.last() == 0) {
    ret.removeLast();
  }
  return ret;
}

bool KGlobalAccelShortcutBackend::RegisterAction(const QString& name,
                                                 QAction* action) {
  Q_ASSERT(action);

  if (name.isEmpty() &&
      (action->objectName().isEmpty() ||
       action->objectName().startsWith(QLatin1String("unnamed-")))) {
    qLog(Warning) << "Cannot register shortcut for unnamed action";
    return false;
  }

  QStringList action_id = GetId(name, action);
  name_to_action_.insertMulti(IdActionUniqueName(action_id), action);
  iface_->doRegister(action_id);

  return true;
}

bool KGlobalAccelShortcutBackend::RegisterShortcut(
    const GlobalShortcuts::Shortcut& shortcut) {
  if (!RegisterAction(shortcut.id, shortcut.action)) return false;

  QList<QKeySequence> active_shortcut;
  active_shortcut << shortcut.action->shortcut();

  QStringList action_id = GetId(shortcut.id, shortcut.action);
  const QList<int> result = iface_->setShortcut(
      action_id, ToIntList(active_shortcut), SetShortcutFlag::SetPresent);

  const QList<QKeySequence> result_sequence = ToKeySequenceList(result);
  if (result_sequence != active_shortcut) {
    qLog(Warning) << "Tried setting global shortcut" << active_shortcut
                  << "but KGlobalAccel returned" << result_sequence;

    if (!result_sequence.isEmpty()) {
      if (!IsCorrectMediaKeyShortcut(shortcut)) {
        // there is some conflict with our preferred shortcut so we use
        // the new shortcut that kglobalaccel suggests
        shortcut.action->setShortcut(result_sequence[0]);
      } else {
        // media keys are properly handled by plasma through the
        // media player plasmoid so we don't do anything in those cases
        qLog(Debug) << "Leaving media key shortcuts unchanged";
      }
    }
  }

  return true;
}

QList<QKeySequence> KGlobalAccelShortcutBackend::ToKeySequenceList(
    const QList<int>& seq) {
  QList<QKeySequence> ret;
  for (int i : seq) {
    ret.append(i);
  }
  return ret;
}

void KGlobalAccelShortcutBackend::UnregisterAction(const QString& name,
                                                   QAction* action) {
  Q_ASSERT(action);

  QStringList action_id = GetId(name, action);
  name_to_action_.remove(IdActionUniqueName(action_id), action);
  iface_->unRegister(action_id);
}

void KGlobalAccelShortcutBackend::OnShortcutPressed(
    const QString& component_unique, const QString& action_unique,
    qlonglong timestamp) const {
  QAction* action = nullptr;
  const QList<QAction*> candidates = name_to_action_.values(action_unique);
  for (QAction* a : candidates) {
    if (ComponentUniqueName() == component_unique) {
      action = a;
    }
  }

  if (action && action->isEnabled()) action->trigger();
}

#endif  // HAVE_DBUS
