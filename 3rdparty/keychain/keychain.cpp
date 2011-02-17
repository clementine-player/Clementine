#include "keychain.h"

#include "default_keychain.h"

#ifdef Q_OS_DARWIN
#include "mac_keychain.h"
#elif defined(Q_OS_UNIX)
#ifndef NO_KWALLET
#include "kwallet_keychain.h"
#endif
#ifndef NO_GNOME_KEYRING
#include "gnome_keychain.h"
#endif
#endif // Q_OS_UNIX

// TODO: Make this configurable.
const QString Keychain::kServiceName = "Google Account";

const Keychain::KeychainDefinition* Keychain::kCompiledImplementations[] = {
#ifdef Q_OS_DARWIN
  new KeychainImpl<MacKeychain>(),
#elif defined(Q_OS_LINUX)
  #ifndef NO_KWALLET
  new KeychainImpl<KWalletKeychain>(),
  #endif
  #ifndef NO_GNOME_KEYRING
  new KeychainImpl<GnomeKeychain>(),
  #endif
#endif
  new KeychainImpl<DefaultKeychain>(),
  NULL
};

Keychain* Keychain::getDefault() {
  const KeychainDefinition** ptr = kCompiledImplementations;
  while (*ptr != NULL) {
    Keychain* keychain = (*ptr)->getInstance();
    if (keychain->isAvailable()) {
      return keychain;
    } else {
      delete keychain;
    }
  }

  return NULL;
}
