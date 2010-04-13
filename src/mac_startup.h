#ifndef MAC_STARTUP_H
#define MAC_STARTUP_H

class GlobalShortcuts;

namespace mac {

void MacMain();
void SetShortcutHandler(GlobalShortcuts* handler);

}  // namespace mac

#endif
