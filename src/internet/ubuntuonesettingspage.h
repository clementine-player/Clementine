#ifndef UBUNTUONESETTINGSPAGE_H
#define UBUNTUONESETTINGSPAGE_H

#include "ui/settingspage.h"

class UbuntuOneAuthenticator;
class UbuntuOneService;
class Ui_UbuntuOneSettingsPage;

class UbuntuOneSettingsPage : public SettingsPage {
  Q_OBJECT
 public:
  UbuntuOneSettingsPage(SettingsDialog* parent = nullptr);

  void Load();
  void Save();

 private slots:
  void LoginClicked();
  void LogoutClicked();
  void Connected(UbuntuOneAuthenticator* authenticator);

 private:
  Ui_UbuntuOneSettingsPage* ui_;
  UbuntuOneService* service_;

  bool authenticated_;
};

#endif  // UBUNTUONESETTINGSPAGE_H
