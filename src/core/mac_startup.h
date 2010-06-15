#ifndef MAC_STARTUP_H
#define MAC_STARTUP_H

class GlobalShortcuts;
class QObject;

class PlatformInterface {
 public:
  // Called when the application should show itself.
  virtual void Activate() = 0;
  //virtual bool LoadUrl(const QString& url) = 0;

  virtual ~PlatformInterface() {}
};

namespace mac {

void MacMain();
void SetShortcutHandler(GlobalShortcuts* handler);
void SetApplicationHandler(PlatformInterface* handler);
void CheckForUpdates();

QString GetBundlePath();
QString GetResourcesPath();

}  // namespace mac

#endif
