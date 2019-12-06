#ifndef CORE_KGLOBALACCELGLOBALSHORTCUTBACKEND_H_
#define CORE_KGLOBALACCELGLOBALSHORTCUTBACKEND_H_

#include "globalshortcutbackend.h"

#include <QSet>
#include <QStringList>

class QAction;

class OrgKdeKGlobalAccelInterface;

class OrgKdeKglobalaccelComponentInterface;


class KGlobalAccelShortcutBackend : public GlobalShortcutBackend {
  Q_OBJECT

  public:
    explicit KGlobalAccelShortcutBackend(GlobalShortcuts *parent);


    static bool isKGlobalAccelAvailable();

  protected:
    bool DoRegister() override;

    void DoUnregister() override;

  private:
#ifdef HAVE_DBUS
    enum SetShortcutFlag {
      SetPresent = 2,
      NoAutoloading = 4,
      IsDefault = 8
    };

    bool acquireComponent();

    bool acquireInterface();

    static QStringList id(const QString &name, const QAction *action);

    static QList<int> intList(const QList<QKeySequence> &seq);

    bool registerAction(const QString &name, QAction *action,
                        QStringList &actionId);

    bool registerShortcut(const GlobalShortcuts::Shortcut &shortcut);

    static QList<QKeySequence> shortcutList(const QList<int> &seq);

    void unregisterAction(const QString &name, QAction *action);

  private slots:

    void onShortcutPressed(const QString &componentUnique,
                           const QString &actionUnique,
                           qlonglong timestamp) const;

  private:
    static const char *Service;
    static const char *Path;

    OrgKdeKGlobalAccelInterface *iface_;
    OrgKdeKglobalaccelComponentInterface *component_;
    QMultiHash<QString, QAction *> nameToAction_;
#endif // HAVE_DBUS
};

#endif //CORE_KGLOBALACCELGLOBALSHORTCUTBACKEND_H_
