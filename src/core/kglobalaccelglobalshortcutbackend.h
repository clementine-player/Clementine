#ifndef CORE_KGLOBALACCELGLOBALSHORTCUTBACKEND_H_
#define CORE_KGLOBALACCELGLOBALSHORTCUTBACKEND_H_

#include "config.h"
#include "globalshortcutbackend.h"
#include "globalshortcuts.h"

#include <QSet>
#include <QStringList>

class QAction;

class OrgKdeKGlobalAccelInterface;

class OrgKdeKglobalaccelComponentInterface;

class KGlobalAccelShortcutBackend : public GlobalShortcutBackend {
  Q_OBJECT

 public:
  explicit KGlobalAccelShortcutBackend(GlobalShortcuts* parent);

  static bool IsKGlobalAccelAvailable();

 protected:
  bool DoRegister() override;

  void DoUnregister() override;

 private:
#ifdef HAVE_DBUS
  enum SetShortcutFlag { SetPresent = 2, NoAutoloading = 4, IsDefault = 8 };

  bool AcquireComponent();

  bool AcquireInterface();

  static QStringList GetId(const QString& name, const QAction* action);

  static QList<int> ToIntList(const QList<QKeySequence>& seq);

  bool RegisterAction(const QString& name, QAction* action);

  bool RegisterShortcut(const GlobalShortcuts::Shortcut& shortcut);

  static QList<QKeySequence> ToKeySequenceList(const QList<int>& seq);

  void UnregisterAction(const QString& name, QAction* action);

 private slots:

  void OnShortcutPressed(const QString& component_unique,
                         const QString& action_unique,
                         qlonglong timestamp) const;

 private:
  static const char* kService;
  static const char* kPath;

  OrgKdeKGlobalAccelInterface* iface_;
  OrgKdeKglobalaccelComponentInterface* component_;
  QMultiHash<QString, QAction*> name_to_action_;
#endif  // HAVE_DBUS
};

#endif  // CORE_KGLOBALACCELGLOBALSHORTCUTBACKEND_H_
