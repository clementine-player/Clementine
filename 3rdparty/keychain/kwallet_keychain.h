#ifndef KWALLET_KEYCHAIN_H
#define KWALLET_KEYCHAIN_H

#include "keychain.h"

#include "kwallet.h"

class KWalletKeychain : public Keychain {
public:
	KWalletKeychain();
	virtual ~KWalletKeychain();

	virtual const QString getPassword(const QString& account);
	virtual bool setPassword(const QString& account, const QString& password);

	virtual bool isAvailable();

	virtual const QString& implementationName() const { return kImplementationName; }

	static void init() {}

	static const QString kImplementationName;
private:
	org::kde::KWallet kwallet_;
	QString wallet_name_;
	int handle_;

	static const QString kKWalletServiceName;
	static const QString kKWalletPath;
	static const QString kKWalletFolder;
};

#endif
