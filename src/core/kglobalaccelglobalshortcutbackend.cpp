#include "config.h"
#include "core/closure.h"
#include "core/logging.h"
#include "globalshortcuts.h"
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
  QString compDisplayName() {
    if (!QGuiApplication::applicationDisplayName().isEmpty()) {
      return QGuiApplication::applicationDisplayName();
    }
    return QCoreApplication::applicationName();
  }

  QString compUniqueName() {
    return QCoreApplication::applicationName();
  }

  const QString &id_ActionUnique(const QStringList &id) {
    return id.at(1);
  }

  bool isCorrectMediaKeyShortcut(const GlobalShortcuts::Shortcut& shortcut) {
    if(shortcut.id == QStringLiteral("play_pause")) {
      return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaPlay);
    } else if(shortcut.id == QStringLiteral("stop")) {
      return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaStop);
    } else if(shortcut.id == QStringLiteral("next_track")) {
      return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaNext);
    } else if(shortcut.id == QStringLiteral("prev_track")) {
      return shortcut.action->shortcut() == QKeySequence(Qt::Key_MediaPrevious);
    } else {
      return false;
    }
  }
}

#ifdef HAVE_DBUS

KGlobalAccelShortcutBackend::KGlobalAccelShortcutBackend(
    GlobalShortcuts *parent)
    : GlobalShortcutBackend(parent),
      iface_(nullptr),
      component_(nullptr),
      nameToAction_() {}

#else // HAVE_DBUS
KGlobalAccelShortcutBackend::KGlobalAccelShortcutBackend(GlobalShortcuts *parent)
  : GlobalShortcutBackend(parent) {}
#endif // HAVE_DBUS

bool KGlobalAccelShortcutBackend::isKGlobalAccelAvailable() {
#ifdef HAVE_DBUS
  return QDBusConnection::sessionBus().interface()->isServiceRegistered(
      Service);
#else // HAVE_DBUS
  return false;
#endif // HAVE_DBUS
}

bool KGlobalAccelShortcutBackend::DoRegister() {
#ifdef HAVE_DBUS
  qLog(Debug) << "Registering shortcuts";

  if (!acquireInterface())
    return false;

  bool complete = true;
  for (const GlobalShortcuts::Shortcut &shortcut : manager_->shortcuts().values()) {
    if(shortcut.action->shortcut().isEmpty())
      continue;

    if (!registerShortcut(shortcut))
      complete = false;
  }

  if (!acquireComponent())
    return false;

  QObject::connect(component_,
                   &OrgKdeKglobalaccelComponentInterface::globalShortcutPressed,
                   this,
                   &KGlobalAccelShortcutBackend::onShortcutPressed);

  return complete;
#else // HAVE_DBUS
  qLog(Warning) << "dbus not available";
  return false;
#endif // HAVE_DBUS
}

void KGlobalAccelShortcutBackend::DoUnregister() {
  if (!acquireInterface())
    return;

  if (!acquireComponent())
    return;

  for (const GlobalShortcuts::Shortcut &shortcut : manager_->shortcuts())
    unregisterAction(shortcut.id, shortcut.action);
}

#ifdef HAVE_DBUS

const char *KGlobalAccelShortcutBackend::Service = "org.kde.kglobalaccel";
const char *KGlobalAccelShortcutBackend::Path = "/kglobalaccel";

bool KGlobalAccelShortcutBackend::acquireComponent() {
  Q_ASSERT(iface_ && iface_->isValid());

  QString componentName = compUniqueName();
  QDBusReply<QDBusObjectPath> reply = iface_->getComponent(compUniqueName());
  if (!reply.isValid()) {
    if (reply.error().name() ==
        QLatin1String("org.kde.kglobalaccel.NoSuchComponent"))
      return false;

    qLog(Warning) << "Failed to get DBus path for KGlobalAccel component";
    return false;
  }

  component_ = new org::kde::kglobalaccel::Component(Service,
                                                     reply.value().path(),
                                                     QDBusConnection::sessionBus(),
                                                     iface_);

  if (!component_->isValid()) {
    qLog(Warning) << "Failed to get KGlobalAccel component:"
                  << QDBusConnection::sessionBus().lastError();
    delete component_;
    component_ = nullptr;
    return false;
  }

  return true;
}

bool KGlobalAccelShortcutBackend::acquireInterface() {
  if (iface_ && iface_->isValid())
    return true;

  if (isKGlobalAccelAvailable()) {
    iface_ = new OrgKdeKGlobalAccelInterface(Service, Path,
                                             QDBusConnection::sessionBus(),
                                             this);
  }

  if (iface_ && iface_->isValid())
    return true;

  if (!iface_)
    qLog(Warning) << "KGlobalAccel daemon not registered";
  else if (!iface_->isValid())
    qLog(Warning) << "KGlobalAccel daemon is not valid";
  return false;
}

QStringList KGlobalAccelShortcutBackend::id(const QString &name,
                                            const QAction *action) {
  Q_ASSERT(action);

  QStringList ret;
  ret << compUniqueName();
  ret << name;
  ret << compDisplayName();
  ret << action->text().replace(QLatin1Char('&'), QStringLiteral(""));
  if (ret.back().isEmpty())
    ret.back() = name;
  return ret;
}

QList<int> KGlobalAccelShortcutBackend::intList(
    const QList<QKeySequence> &seq) {
  QList<int> ret;
  for (const QKeySequence &sequence : seq) {
    ret.append(sequence[0]);
  }
  while (!ret.isEmpty() && ret.last() == 0) {
    ret.removeLast();
  }
  return ret;
}

bool KGlobalAccelShortcutBackend::registerAction(const QString &name,
                                                 QAction *action,
                                                 QStringList &actionId) {
  Q_ASSERT(action);

  if (name.isEmpty() && (action->objectName().isEmpty() ||
                         action->objectName().startsWith(
                             QLatin1String("unnamed-")))) {
    qLog(Warning) << "Cannot register shortcut for unnamed action";
    return false;
  }

  actionId = id(name, action);
  nameToAction_.insertMulti(id_ActionUnique(actionId), action);
  iface_->doRegister(actionId);

  return true;
}

bool KGlobalAccelShortcutBackend::registerShortcut(const GlobalShortcuts::Shortcut &shortcut) {
  QStringList actionId;
  if (!registerAction(shortcut.id, shortcut.action, actionId))
    return false;

  QList<QKeySequence> activeShortcut;
  activeShortcut << shortcut.action->shortcut();

  const QList<int> result = iface_->setShortcut(actionId,
                                                intList(activeShortcut),
                                                SetShortcutFlag::SetPresent);

  const QList<QKeySequence> resultSequence = shortcutList(result);
  if (resultSequence != activeShortcut) {
    qLog(Warning) << "Tried setting global shortcut" << activeShortcut
                  << "but KGlobalAccel returned" << resultSequence;

    if (!resultSequence.isEmpty()) {
      if(!isCorrectMediaKeyShortcut(shortcut)) {
        // there is some conflict with our preferred shortcut so we use
        // the new shortcut that kglobalaccel suggests
        shortcut.action->setShortcut(resultSequence[0]);
      } else {
        // media keys are properly handled by plasma through the
        // media player plasmoid so we don't do anything in those cases
        qLog(Debug) << "Leaving media key shortcuts unchanged";
      }
    }
  }

  return true;
}

QList<QKeySequence> KGlobalAccelShortcutBackend::shortcutList(
    const QList<int> &seq) {
  QList<QKeySequence> ret;
  for (int i : seq) {
    ret.append(i);
  }
  return ret;
}

void KGlobalAccelShortcutBackend::unregisterAction(const QString &name,
                                                   QAction *action) {
  Q_ASSERT(action);

  QStringList actionId = id(name, action);
  nameToAction_.remove(id_ActionUnique(actionId), action);
  iface_->unRegister(actionId);
}

void KGlobalAccelShortcutBackend::onShortcutPressed(
    const QString &componentUnique,
    const QString &actionUnique,
    qlonglong timestamp) const {
  QAction *action = nullptr;
  const QList<QAction *> candidates = nameToAction_.values(actionUnique);
  for (QAction *a : candidates) {
    if (compUniqueName() == componentUnique) {
      action = a;
    }
  }

  if (action && action->isEnabled())
    action->trigger();
}

#endif // HAVE_DBUS
