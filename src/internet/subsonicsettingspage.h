#ifndef SUBSONICSETTINGSPAGE_H
#define SUBSONICSETTINGSPAGE_H

#include "ui/settingspage.h"
#include "subsonicservice.h"

class Ui_SubsonicSettingsPage;

class SubsonicSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  SubsonicSettingsPage(SettingsDialog* dialog);
  ~SubsonicSettingsPage();

  void Load();
  void Save();

 public slots:
  void LoginStateChanged(SubsonicService::LoginState newstate);

 private slots:
  void ServerEditingFinished();
  void Login();
  void Logout();

 private:
  Ui_SubsonicSettingsPage* ui_;
  SubsonicService* service_;
};

#endif  // SUBSONICSETTINGSPAGE_H
