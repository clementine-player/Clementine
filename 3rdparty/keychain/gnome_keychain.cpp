#include "gnome_keychain.h"

#include <glib.h>

#include <QCoreApplication>
#include <QtDebug>

const QString GnomeKeychain::kImplementationName = "Gnome Keyring";

const GnomeKeyringPasswordSchema GnomeKeychain::kOurSchema = {
  GNOME_KEYRING_ITEM_GENERIC_SECRET,
  {
    { "username", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
    { "service",  GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
    { NULL },
  },
};

bool GnomeKeychain::isAvailable() {
  return gnome_keyring_is_available();
}

const QString GnomeKeychain::getPassword(const QString& account) {
  Q_ASSERT(isAvailable());
  char* password;
  GnomeKeyringResult result = gnome_keyring_find_password_sync(
    &kOurSchema,
    &password,
    "username", account.toUtf8().constData(),
    "service", kServiceName.toUtf8().constData(),
    NULL);

  if (result == GNOME_KEYRING_RESULT_OK) {
    QString pass(password);
    gnome_keyring_free_password(password);
    return pass;
  }

  qWarning() << "Failed to get password from keychain for account" << account;
  return QString::null;
}

bool GnomeKeychain::setPassword(const QString& account, const QString& password) {
  Q_ASSERT(isAvailable());
  QString displayName = QString("%1 account for %2").arg(
        QCoreApplication::applicationName(), account);

  GnomeKeyringResult result = gnome_keyring_store_password_sync(
    &kOurSchema,
    NULL,
    displayName.toUtf8().constData(),
    password.toUtf8().constData(),
    "username", account.toUtf8().constData(),
    "service", kServiceName.toUtf8().constData(),
    NULL);

  return result == GNOME_KEYRING_RESULT_OK;
}
