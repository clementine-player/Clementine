#!/usr/bin/env python3
"""Claude Code PostToolUse hook that reformats a C++ file Claude just edited,
with the same clang-format version, style and scope (src/ only) as the lint
job in .github/workflows/all.yml.
"""
import json
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'dist'))
import format as format_script  # dist/format.py


def main():
  path = json.load(sys.stdin).get('tool_input', {}).get('file_path')
  if not path or not os.path.exists(path):
    return 0

  try:
    rel = os.path.relpath(os.path.abspath(path), ROOT).replace(os.sep, '/')
  except ValueError:
    # On a different drive to the repository.
    return 0
  extension = os.path.splitext(rel)[1][1:]
  if not rel.startswith('src/') or extension not in format_script.EXTENSIONS:
    return 0

  # Exit code 2 shows these messages to Claude, so it can tell the user.
  version = format_script.CLANG_FORMAT_VERSION
  install = ('Install it with `uv tool install clang-format==%s` (or pipx), '
             'which puts it on PATH.' % version)
  executable = shutil.which('clang-format')
  if executable is None:
    print('clang-format is not on PATH, so %s was not formatted and CI\'s '
          'lint job may fail. %s' % (rel, install), file=sys.stderr)
    return 2

  version_output = subprocess.run(
      [executable, '--version'], capture_output=True, text=True).stdout
  if ('version ' + version) not in version_output:
    print('%s is %r, but CI uses clang-format %s, which formats some code '
          'differently, so %s was not formatted. %s' % (
              executable, version_output.strip(), version, rel, install),
          file=sys.stderr)
    return 2

  with open(path, 'rb') as fh:
    original = fh.read()
  subprocess.run(
      [executable, '-i', '-style=' + format_script.STYLE, path], check=True)
  with open(path, 'rb') as fh:
    changed = fh.read() != original

  if changed:
    # Tell Claude, so it re-reads the file before editing it again.
    json.dump({'hookSpecificOutput': {
        'hookEventName': 'PostToolUse',
        'additionalContext': 'clang-format reformatted %s.' % rel,
    }}, sys.stdout)
  return 0


if __name__ == '__main__':
  sys.exit(main())
