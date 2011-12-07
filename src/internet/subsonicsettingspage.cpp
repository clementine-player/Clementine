#include "subsonicsettingspage.h"
#include "ui_subsonicsettingspage.h"
#include "subsonicservice.h"

#include <QSettings>

SubsonicSettingsPage::SubsonicSettingsPage(SettingsDialog *dialog)
  : SettingsPage(dialog),
    ui_(new Ui_SubsonicSettingsPage)
{
  ui_->setupUi(this);

  setWindowIcon(QIcon(":/providers/subsonic-32.png"));
}

SubsonicSettingsPage::~SubsonicSettingsPage()
{
    delete ui_;
}

void SubsonicSettingsPage::Load()
{
  QSettings s;
  s.beginGroup(SubsonicService::kSettingsGroup);

  ui_->server->setText(s.value("server").toString());
  ui_->username->setText(s.value("username").toString());
  ui_->password->setText(s.value("password").toString());
}

void SubsonicSettingsPage::Save()
{
  QSettings s;
  s.beginGroup(SubsonicService::kSettingsGroup);

  s.setValue("server", ui_->server->text());
  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());
}
