#ifndef DEFAULT_KEYCHAIN_H
#define DEFAULT_KEYCHAIN_H

#include "keychain.h"

class DefaultKeychain : public Keychain {
public:
	virtual ~DefaultKeychain() {}
	virtual bool isAvailable() { return true; }

	virtual const QString getPassword(const QString& account);
	virtual bool setPassword(const QString& account, const QString& password);

	virtual const QString& implementationName() const { return kImplementationName; }

	static void init() {}

	static const QString kImplementationName;
private:
	QString password_;
};

#endif
