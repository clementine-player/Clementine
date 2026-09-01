#!/usr/bin/env python3
"""Reformats C++ source files that have changed from a given git ref, using
the same clang-format style and scope as .github/workflows/all.yml's
clang-format-lint-action.
"""
import argparse
import difflib
import os
import shutil
import subprocess
import sys


def main():
  parser = argparse.ArgumentParser(
      description='Reformats C++ source files that have changed from a given '
                  'git ref.')
  parser.add_argument('--clang-format-executable', default='clang-format',
      help='path to the clang-format executable')
  parser.add_argument('--ref', default='origin/master',
      help='the git-ref to compare against')
  parser.add_argument('--source', default='src',
      help='only reformat files under this directory (matches CI\'s scope)')
  parser.add_argument('--extension', action='append', metavar='EXT',
      default=['c', 'h', 'C', 'H', 'cpp', 'hpp', 'cc', 'hh', 'c++', 'h++',
                'cxx', 'hxx'],
      help='file extensions to reformat')
  parser.add_argument('-i', dest='inplace', action='store_true',
      help='edit files inplace instead of showing a diff')
  parser.add_argument('--files', nargs='*', metavar='FIL',
      default=[],
      help='get files as arguments instead of git')
  args = parser.parse_args()

  if shutil.which(args.clang_format_executable) is None:
    print('%s: could not find clang-format executable %r - install it '
          '(eg. `brew install clang-format`) or pass '
          '--clang-format-executable' % (
              sys.argv[0], args.clang_format_executable), file=sys.stderr)
    return 1

  try:
    root_dir = subprocess.check_output(
        ['git', 'rev-parse', '--show-toplevel'],
        text=True).strip()
  except subprocess.CalledProcessError:
    # Probably we were not called from a git working directory, just ignore
    # this error.
    return

  changed_files = subprocess.check_output(
      ['git', 'diff-index', args.ref, '--name-only'],
      text=True).splitlines()
  source_prefix = args.source.rstrip('/') + '/'
  changed_files = [f for f in changed_files if f.startswith(source_prefix)]

  if not changed_files:
    print('No changes from %s' % args.ref, file=sys.stderr)

  if not args.files and not changed_files:
    print('Use --files to select files for reformat', file=sys.stderr)
    return

  if args.files:
    changed_files = args.files

  style = '{BasedOnStyle: Google, DerivePointerBinding: false, Standard: Cpp11}'

  had_diff = False

  for filename in changed_files:
    if not os.path.splitext(filename)[1][1:] in args.extension:
      continue

    path = os.path.join(root_dir, filename)
    if not os.path.exists(path):
      # Probably a deletion
      continue

    with open(path, encoding='utf-8') as fh:
      original = fh.read()

    formatted = subprocess.run(
        [args.clang_format_executable, '-style=' + style, path],
        capture_output=True, text=True, check=True).stdout

    if original == formatted:
      print('%s: formatting is correct!' % filename, file=sys.stderr)
      continue

    diff = list(difflib.unified_diff(
        original.split('\n'), formatted.split('\n'),
        os.path.join('a', filename), os.path.join('b', filename),
        lineterm=''))

    if args.inplace:
      with open(path, 'w', encoding='utf-8') as fh:
        fh.write(formatted)

      print('%s: %d insertion(s), %d deletion(s)' % (
          filename,
          sum(1 for x in diff if x.startswith('+')),
          sum(1 for x in diff if x.startswith('-'))), file=sys.stderr)
    else:
      had_diff = True
      print('\n'.join(diff))

  return 1 if had_diff else 0


if __name__ == '__main__':
  sys.exit(main() or 0)
