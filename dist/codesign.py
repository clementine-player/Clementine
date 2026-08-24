#!/usr/bin/env python3

# Signs every Mach-O binary inside a .app bundle with hardened runtime enabled,
# innermost-first, then signs the bundle itself. codesign --deep exists on modern
# macOS but signs nested code with the outer bundle's (default, non-hardened)
# options; walking the tree ourselves lets every nested binary get the hardened
# runtime + entitlements it needs to pass notarization.

import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_ENTITLEMENTS = os.path.join(SCRIPT_DIR, 'entitlements.plist')


def IsMachO(path):
  if os.path.islink(path):
    return False
  try:
    output = subprocess.check_output(['file', '-b', path]).decode('utf-8')
  except subprocess.CalledProcessError:
    return False
  return 'Mach-O' in output


def FindSignablePaths(app_bundle):
  """Returns paths to sign, deepest/innermost first."""
  nested_bundles = []
  binaries = []

  for root, dirs, files in os.walk(app_bundle):
    for d in sorted(dirs):
      if d.endswith(('.framework', '.app')):
        nested_bundles.append(os.path.join(root, d))
        # Don't descend into it separately; it's signed as a unit below by
        # re-walking it, but we still want its own internal binaries found,
        # so keep walking - just record it as a bundle to codesign directly
        # afterwards.
    for f in sorted(files):
      path = os.path.join(root, f)
      if IsMachO(path):
        binaries.append(path)

  # Sign plain binaries/dylibs first (most deeply nested by construction of
  # os.walk), then frameworks/nested apps deepest-path-first so an inner
  # framework is signed before the outer bundle that contains it.
  nested_bundles.sort(key=lambda p: p.count(os.sep), reverse=True)
  return binaries + nested_bundles


def SignPath(path, developer_id, entitlements):
  args = ['codesign', '--force']
  if developer_id != '-':
    # Hardened runtime and secure timestamps only make sense (and only
    # work) with a real signing identity - ad-hoc signing is just there to
    # satisfy Apple Silicon's "every loaded binary needs *a* valid
    # signature" requirement for local runs.
    args += ['--options', 'runtime', '--timestamp']
  args += ['-s', developer_id, '--entitlements', entitlements, '-v', path]
  subprocess.check_call(args)


def main():
  if len(sys.argv) not in (3, 4):
    print('Usage: %s <developer id> <app bundle> [entitlements.plist]' %
          sys.argv[0])
    sys.exit(1)

  developer_id = sys.argv[1]
  app_bundle = sys.argv[2]
  entitlements = sys.argv[3] if len(sys.argv) > 3 else DEFAULT_ENTITLEMENTS

  for path in FindSignablePaths(app_bundle):
    SignPath(path, developer_id, entitlements)

  SignPath(app_bundle, developer_id, entitlements)

  # Verify the signature is valid. Note spctl won't accept it yet at this
  # point since Gatekeeper also requires a notarization ticket - that's
  # checked separately after the notarize step staples one on.
  subprocess.check_call(['codesign', '--verify', '--deep', '--strict',
                          '--verbose=4', app_bundle])


if __name__ == '__main__':
  main()
