#!/usr/bin/python
# Emulates the behaviour of codesign --deep which is missing on OS X < 10.9

import os
import re
import subprocess
import sys

def SignPath(path, developer_id, deep=True):
  args = [
    'codesign',
    '--preserve-metadata=identifier,entitlements,resource-rules,requirements',
    '-s', developer_id,
    '-fv', path
  ]
  if deep:
    args.append('--deep')
  subprocess.check_call(args)

def main():
  if len(sys.argv) != 3:
    print 'Usage: %s <developer id> <app bundle>' % sys.argv[0]
    sys.exit(1)

  developer_id = sys.argv[1]
  app_bundle = sys.argv[2]

  for root, dirs, files in os.walk(app_bundle):
    for dir in dirs:
      if re.search(r'\.framework$', dir):
        SignPath(os.path.join(root, dir), developer_id)

    for file in files:
      if re.search(r'\.(dylib|so)$', file):
        SignPath(os.path.join(root, file), developer_id)
      elif re.match(r'(clementine-spotifyblob|clementine-tagreader|gst-plugin-scanner)', file):
        SignPath(os.path.join(root, file), developer_id)

  SignPath(app_bundle, developer_id, deep=False)

  # Verify the signatures are valid.
  subprocess.check_call([
      'codesign', '--verify', '--verbose=4', app_bundle])
  subprocess.check_call([
      'spctl', '--assess', '--verbose=4', app_bundle])


if __name__ == '__main__':
  main()
