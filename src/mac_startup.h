#ifndef MAC_STARTUP_H
#define MAC_STARTUP_H

class GlobalShortcuts;
class QObject;

namespace mac {

void MacMain();
void SetShortcutHandler(GlobalShortcuts* handler);
void SetApplicationHandler(QObject* handler);

}  // namespace mac

#endif
