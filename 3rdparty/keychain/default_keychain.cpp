#include "default_keychain.h"

const QString DefaultKeychain::kImplementationName = "Default";

const QString DefaultKeychain::getPassword(const QString& account) {
	Q_UNUSED(account);
	return password_;
}

bool DefaultKeychain::setPassword(const QString& account, const QString& password) {
	Q_UNUSED(account);
	password_ = password;
	return true;
}
