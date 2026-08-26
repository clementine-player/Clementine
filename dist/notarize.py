#!/usr/bin/env python3

# Submits a signed .app or .dmg to Apple's notary service and staples the
# resulting ticket on success, so Gatekeeper can verify it offline.
#
# Takes the path to an App Store Connect API key JSON file (the
# {"key_id", "issuer_id", "key"} shape stored in the
# apple-appstoreconnect-api-key GCP secret and used by fastlane match - see
# dist/setup_signing.sh) as this script's first argument / the
# APPLE_NOTARIZE_API_KEY_PATH cmake cache variable. No separate `xcrun
# notarytool store-credentials` setup is needed.

import json
import os
import subprocess
import sys
import tempfile


def Notarize(path, api_key_path):
  submit_path = path
  zip_path = None

  if path.endswith('.app'):
    # notarytool only accepts zips/dmgs/pkgs, not raw .app bundles.
    fd, zip_path = tempfile.mkstemp(suffix='.zip')
    os.close(fd)
    os.remove(zip_path)
    subprocess.check_call(
        ['ditto', '-c', '-k', '--keepParent', path, zip_path])
    submit_path = zip_path

  with open(api_key_path) as f:
    api_key = json.load(f)

  # tempfile.mkstemp's file is already mode 0600 (owner-only) by default.
  key_fd, key_path = tempfile.mkstemp(suffix='.p8')
  os.write(key_fd, api_key['key'].encode('utf-8'))
  os.close(key_fd)

  try:
    subprocess.check_call([
        'xcrun', 'notarytool', 'submit', submit_path,
        '--key', key_path,
        '--key-id', api_key['key_id'],
        '--issuer', api_key['issuer_id'],
        '--wait',
    ])
  finally:
    os.remove(key_path)
    if zip_path:
      os.remove(zip_path)

  subprocess.check_call(['xcrun', 'stapler', 'staple', path])
  if path.endswith('.app'):
    subprocess.check_call(['spctl', '--assess', '--verbose=4', path])


def main():
  if len(sys.argv) != 3:
    print('Usage: %s <API key JSON path> <app bundle or dmg>' % sys.argv[0])
    sys.exit(1)

  Notarize(sys.argv[2], sys.argv[1])


if __name__ == '__main__':
  main()
