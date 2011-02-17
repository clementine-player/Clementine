#include "mac_keychain.h"

#include <Security/Security.h>

const QString MacKeychain::kImplementationName = "Mac Keychain";

bool MacKeychain::isAvailable() {
  return true;
}

const QString MacKeychain::getPassword(const QString& account) {
  UInt32 password_length;
  char* password;
  OSStatus ret = SecKeychainFindGenericPassword(
    NULL,
    kServiceName.length(),
    kServiceName.toStdString().c_str(),
    account.length(),
    account.toStdString().c_str(),
    &password_length,
    (void**)&password,
    NULL);

  if (ret == 0) {
    QString pass = QString::fromAscii(password, password_length);
    SecKeychainItemFreeContent(NULL, password);
    return pass;
  }

  return QString::null;
}

bool MacKeychain::setPassword(const QString& account, const QString& password) {
  SecKeychainItemRef item;
  OSStatus ret = SecKeychainFindGenericPassword(
    NULL,
    kServiceName.length(),
    kServiceName.toStdString().c_str(),
    account.length(),
    account.toStdString().c_str(),
    NULL,
    NULL,
    &item);

  if (ret == 0) {
    ret = SecKeychainItemModifyAttributesAndData(
      item,
      NULL,
      password.length(),
      password.toStdString().c_str());

    return ret == 0;
  } else {
    ret = SecKeychainAddGenericPassword(
      NULL,
      kServiceName.length(),
      kServiceName.toStdString().c_str(),
      account.length(),
      account.toStdString().c_str(),
      password.length(),
      password.toStdString().c_str(),
      NULL);

    return ret == 0;
  }
}
