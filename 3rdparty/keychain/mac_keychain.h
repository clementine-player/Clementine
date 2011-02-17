#ifndef MAC_KEYCHAIN_H
#define MAC_KEYCHAIN_H

#include "keychain.h"

class MacKeychain : public Keychain {
public:
	virtual ~MacKeychain() {}
	virtual bool isAvailable();

	virtual const QString getPassword(const QString& account);
	virtual bool setPassword(const QString& account, const QString& password);

	virtual const QString& implementationName() const { return kImplementationName; }

	static const QString kImplementationName;

	static void init() {}
};

#endif
