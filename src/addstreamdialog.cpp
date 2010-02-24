#include "addstreamdialog.h"
#include "radiomodel.h"
#include "savedradio.h"

#include <QSettings>
#include <QUrl>
#include <QPushButton>
#include <QtDebug>

const char* AddStreamDialog::kSettingsGroup = "AddStreamDialog";

AddStreamDialog::AddStreamDialog(QWidget *parent)
  : QDialog(parent),
    saved_radio_(NULL)
{
  ui_.setupUi(this);

  connect(ui_.url, SIGNAL(textChanged(QString)), SLOT(TextChanged(QString)));
  TextChanged(QString::null);

  // Restore settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui_.save->setChecked(s.value("save", true).toBool());
  ui_.url->setText(s.value("url").toString());

  // Connections to the saved streams service
  saved_radio_ = qobject_cast<SavedRadio*>(
      RadioModel::ServiceByName(SavedRadio::kServiceName));

  connect(saved_radio_, SIGNAL(ShowAddStreamDialog()), SLOT(show()));
}

QUrl AddStreamDialog::url() const {
  return QUrl(ui_.url->text());
}

void AddStreamDialog::accept() {
  if (ui_.save->isChecked()) {
    saved_radio_->Add(url());
  }

  // Save settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("save", ui_.save->isChecked());
  s.setValue("url", url().toString());

  QDialog::accept();
}

void AddStreamDialog::TextChanged(const QString &text) {
  // Decide whether the URL is valid
  QUrl url(text);

  bool valid = url.isValid() &&
               !url.scheme().isEmpty() &&
               !url.toString().isEmpty();

  ui_.button_box->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

