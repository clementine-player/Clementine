#ifndef GNOME_KEYCHAIN_H
#define GNOME_KEYCHAIN_H

#include "keychain.h"

extern "C" {
#include <gnome-keyring.h>
}

class GnomeKeychain : public Keychain {
public:
	virtual ~GnomeKeychain() {}
	virtual bool isAvailable();
	virtual const QString getPassword(const QString& account);
	virtual bool setPassword(const QString& account, const QString& password);

	virtual const QString& implementationName() const { return kImplementationName; }

	static void init();

	static const QString kImplementationName;
private:
	static const GnomeKeyringPasswordSchema kOurSchema;
};

#endif
