#include "config.h"
#include "gnome_keychain.h"

#include <glib.h>

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
		"username", account.toStdString().c_str(),
		"service", kServiceName.toStdString().c_str(),
		NULL);
	
	if (result == GNOME_KEYRING_RESULT_OK) {
		QString pass(password);
		gnome_keyring_free_password(password);
		return pass;
	}

	return QString::null;
}

bool GnomeKeychain::setPassword(const QString& account, const QString& password) {
	Q_ASSERT(isAvailable());
	QString displayName = "%1 Google Reader account for %2";
	displayName.arg(TITLE);
	displayName.arg(account);

	GnomeKeyringResult result = gnome_keyring_store_password_sync(
		&kOurSchema,
		NULL,
		displayName.toStdString().c_str(),
		password.toStdString().c_str(),
		"username", account.toStdString().c_str(),
		"service", kServiceName.toStdString().c_str(),
		NULL);
	
	return result == GNOME_KEYRING_RESULT_OK;
}

void GnomeKeychain::init() {
	g_set_application_name("PurpleHatstands");
}
