#ifndef MAC_STARTUP_H
#define MAC_STARTUP_H

class GlobalShortcuts;
class QObject;

namespace mac {

void MacMain();
void SetShortcutHandler(GlobalShortcuts* handler);
void SetApplicationHandler(QObject* handler);
void CheckForUpdates();

QString GetBundlePath();
QString GetResourcesPath();

}  // namespace mac

#endif
