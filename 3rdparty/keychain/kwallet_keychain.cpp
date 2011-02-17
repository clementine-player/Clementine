#include "kwallet_keychain.h"

#include <QDBusConnection>
#include <QDBusPendingReply>

const QString KWalletKeychain::kImplementationName = "KWallet";
const QString KWalletKeychain::kKWalletServiceName = "org.kde.kwalletd";
const QString KWalletKeychain::kKWalletPath = "/modules/kwalletd";
const QString KWalletKeychain::kKWalletFolder = "Passwords";

KWalletKeychain::KWalletKeychain()
	: kwallet_(kKWalletServiceName, kKWalletPath, QDBusConnection::sessionBus()) {
	if (isAvailable()) {
		QDBusPendingReply<QString> wallet_name = kwallet_.networkWallet();
		wallet_name.waitForFinished();
		if (wallet_name.isValid()) {
			wallet_name_ = wallet_name.value();
			QDBusPendingReply<int> open_request = kwallet_.open(wallet_name_, 0, kServiceName);
			open_request.waitForFinished();
			if (open_request.isValid()) {
				handle_ = open_request.value();
			}
		}
	}
}

KWalletKeychain::~KWalletKeychain() {
}

bool KWalletKeychain::isAvailable() {
	if(!kwallet_.isValid())
		return false;
	
	QDBusPendingReply<bool> check = kwallet_.isEnabled();
	check.waitForFinished();
	return check.isValid() && check.value();
}

const QString KWalletKeychain::getPassword(const QString& account) {
	QDBusPendingReply<QString> password =
		kwallet_.readPassword(handle_, kKWalletFolder, account, kServiceName);
	password.waitForFinished();
	if (password.isValid()) {
		return password.value();
	}

	return QString::null;
}

bool KWalletKeychain::setPassword(const QString& account, const QString& password) {
	QDBusPendingReply<int> ret =
		kwallet_.writePassword(handle_, kKWalletFolder, account, password, kServiceName);
	ret.waitForFinished();
	if (ret.isValid() && ret.value() == 0) {
		return true;
	}

	return false;
}
