#!/usr/bin/env python3

# Submits a signed .app or .dmg to Apple's notary service and staples the
# resulting ticket on success, so Gatekeeper can verify it offline.
#
# Requires a notarytool keychain profile to already be configured, e.g.:
#   xcrun notarytool store-credentials <profile-name> \
#       --apple-id you@example.com --team-id TEAMID --password app-specific-password
# (or an App Store Connect API key stored the same way). The profile name is
# passed as this script's first argument / the APPLE_NOTARIZE_PROFILE cmake
# cache variable.

import os
import subprocess
import sys
import tempfile


def Notarize(path, keychain_profile):
  submit_path = path
  cleanup = None

  if path.endswith('.app'):
    # notarytool only accepts zips/dmgs/pkgs, not raw .app bundles.
    fd, zip_path = tempfile.mkstemp(suffix='.zip')
    os.close(fd)
    os.remove(zip_path)
    subprocess.check_call(
        ['ditto', '-c', '-k', '--keepParent', path, zip_path])
    submit_path = zip_path
    cleanup = zip_path

  try:
    subprocess.check_call([
        'xcrun', 'notarytool', 'submit', submit_path,
        '--keychain-profile', keychain_profile,
        '--wait',
    ])
  finally:
    if cleanup:
      os.remove(cleanup)

  subprocess.check_call(['xcrun', 'stapler', 'staple', path])
  if path.endswith('.app'):
    subprocess.check_call(['spctl', '--assess', '--verbose=4', path])


def main():
  if len(sys.argv) != 3:
    print('Usage: %s <keychain profile> <app bundle or dmg>' % sys.argv[0])
    sys.exit(1)

  Notarize(sys.argv[2], sys.argv[1])


if __name__ == '__main__':
  main()
