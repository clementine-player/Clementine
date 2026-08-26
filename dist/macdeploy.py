#!/usr/bin/python3

#  This file is part of Clementine.
#
#  Clementine is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Clementine is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Clementine.  If not, see <http://www.gnu.org/licenses/>.

import logging
import os
import re
import shutil
import subprocess
import sys
import traceback

LOGGER = logging.getLogger('macdeploy')


def GetHomebrewPrefixes():
  # Apple Silicon uses /opt/homebrew, Intel uses /usr/local; support both so
  # this script doesn't need to know which one built the dependencies.
  prefixes = []
  env_prefix = os.environ.get('HOMEBREW_PREFIX')
  if env_prefix:
    prefixes.append(env_prefix)
  for candidate in ('/opt/homebrew', '/usr/local'):
    if candidate not in prefixes and os.path.isdir(candidate):
      prefixes.append(candidate)
  return prefixes or ['/usr/local']


HOMEBREW_PREFIXES = GetHomebrewPrefixes()

FRAMEWORK_SEARCH_PATH = [
    '/target', '/target/lib', '/Library/Frameworks',
    os.path.join(os.environ['HOME'], 'Library/Frameworks')
] + [
    # Homebrew's "qt" formula symlinks every Qt module's framework bundle
    # into its own lib/ dir (e.g. opt/qt/lib/QtCore.framework), regardless
    # of which per-module formula (qtbase, qtsvg, ...) actually owns it.
    os.path.join(prefix, 'opt/qt/lib') for prefix in HOMEBREW_PREFIXES
]

STRIP_PREFIX = [
    '@@HOMEBREW_PREFIX@@/opt/qt5/lib/',
    '@@HOMEBREW_CELLAR@@/qt5/5.8.0_1/lib/',
    '@rpath/',
]

LIBRARY_SEARCH_PATH = ['/target', '/target/lib', '/sw/lib'] + [
    os.path.join(prefix, 'lib') for prefix in HOMEBREW_PREFIXES
]

GSTREAMER_PLUGINS = [
    # Core plugins
    'libgstapp.dylib',
    'libgstaudioconvert.dylib',
    'libgstaudiofx.dylib',
    'libgstaudiotestsrc.dylib',
    'libgstaudioresample.dylib',
    'libgstautodetect.dylib',
    'libgstcoreelements.dylib',
    'libgstequalizer.dylib',
    'libgstgdp.dylib',
    'libgstosxaudio.dylib',
    'libgstplayback.dylib',
    'libgsttcp.dylib',
    'libgsttypefindfunctions.dylib',
    'libgstudp.dylib',
    'libgstvolume.dylib',

    # Codecs
    'libgstapetag.dylib',
    'libgstasf.dylib',
    'libgstaudioparsers.dylib',
    #'libgstfaac.dylib',
    'libgstfaad.dylib',
    'libgstflac.dylib',
    'libgstid3demux.dylib',
    'libgstisomp4.dylib',
    'libgstlame.dylib',
    'libgstlibav.dylib',
    # TODO: Bring back Musepack support.
    'libgstogg.dylib',
    'libgstopus.dylib',
    'libgstreplaygain.dylib',
    'libgstspeex.dylib',
    'libgsttaglib.dylib',
    'libgstvorbis.dylib',
    'libgstwavparse.dylib',
    'libgstxingmux.dylib',

    # HTTP src support
    'libgstsoup.dylib',

    # Icecast support
    'libgsticydemux.dylib',

    # RTSP streaming
    'libgstrtp.dylib',
    'libgstrtsp.dylib',
]

GSTREAMER_SEARCH_PATH = [
    '/target/lib/gstreamer-1.0',
    '/target/libexec/gstreamer-1.0',
]
for _prefix in HOMEBREW_PREFIXES:
  GSTREAMER_SEARCH_PATH += [
      os.path.join(_prefix, 'lib/gstreamer-1.0'),
      os.path.join(_prefix, 'opt/gstreamer/lib/gstreamer-1.0'),
      os.path.join(_prefix, 'opt/gstreamer/libexec/gstreamer-1.0'),
  ]

QT_PLUGINS = [
    #'accessible/libqtaccessiblewidgets.dylib',
    #'codecs/libqcncodecs.dylib',
    #'codecs/libqjpcodecs.dylib',
    #'codecs/libqkrcodecs.dylib',
    #'codecs/libqtwcodecs.dylib',
    'iconengines/libqsvgicon.dylib',
    'imageformats/libqgif.dylib',
    'imageformats/libqico.dylib',
    'imageformats/libqjpeg.dylib',
    #'imageformats/libqmng.dylib',
    'imageformats/libqsvg.dylib',
    'platforms/libqcocoa.dylib',
    'styles/libqmacstyle.dylib',
]
QT_PLUGINS_SEARCH_PATH = ['/target/plugins']
for _prefix in HOMEBREW_PREFIXES:
  QT_PLUGINS_SEARCH_PATH += [
      # Homebrew's Qt6 formulas each keep their own plugins under their own
      # keg's share/qt/plugins (e.g. qtbase owns platforms/styles/most
      # imageformats, qtsvg owns iconengines/svg) rather than one aggregated
      # plugins/ dir like Qt5's qt@5 formula had.
      os.path.join(_prefix, 'opt/qtbase/share/qt/plugins'),
      os.path.join(_prefix, 'opt/qtsvg/share/qt/plugins'),
      os.path.join(_prefix, 'opt/qt@5/plugins'),
      os.path.join(_prefix, 'opt/qt5/plugins'),
  ]

GIO_MODULES_SEARCH_PATH = ['/target/lib/gio/modules'] + [
    os.path.join(prefix, 'lib/gio/modules') for prefix in HOMEBREW_PREFIXES
]

INSTALL_NAME_TOOL_APPLE = 'install_name_tool'
INSTALL_NAME_TOOL_CROSS = 'x86_64-apple-darwin-%s' % INSTALL_NAME_TOOL_APPLE
INSTALL_NAME_TOOL = INSTALL_NAME_TOOL_CROSS if shutil.which(
    INSTALL_NAME_TOOL_CROSS) else INSTALL_NAME_TOOL_APPLE

OTOOL_APPLE = 'otool'
OTOOL_CROSS = 'x86_64-apple-darwin-%s' % OTOOL_APPLE
OTOOL = OTOOL_CROSS if shutil.which(OTOOL_CROSS) else OTOOL_APPLE


class Error(Exception):
  pass


class CouldNotFindFrameworkError(Error):
  pass


class InstallNameToolError(Error):
  pass


class CouldNotFindXinePluginError(Error):
  pass


class CouldNotFindQtPluginError(Error):
  pass


class CouldNotFindGstreamerPluginError(Error):
  pass


class CouldNotFindGioModuleError(Error):
  pass


class CouldNotParseFrameworkNameError(Error):
  pass


if len(sys.argv) < 2:
  print('Usage: %s <bundle.app>' % sys.argv[0])

bundle_dir = sys.argv[1]

bundle_name = os.path.basename(bundle_dir).split('.')[0]

commands = []

frameworks_dir = os.path.join(bundle_dir, 'Contents', 'Frameworks')
resources_dir = os.path.join(bundle_dir, 'Contents', 'Resources')
plugins_dir = os.path.join(bundle_dir, 'Contents', 'PlugIns')
binary = os.path.join(bundle_dir, 'Contents', 'MacOS', bundle_name)

# This script's own commands (cp/ln -sf) aren't idempotent against a
# Frameworks/PlugIns tree it already populated on a previous run: copying a
# framework's Resources into a destination that already has one, or
# re-linking Versions/Current on top of an existing chain, can produce
# self-referential entries (e.g. Versions/A/A) or symlink loops ("too many
# levels of symbolic links") instead of just overwriting cleanly. Wipe and
# recreate them fresh on every run rather than trying to merge into
# whatever's already there. Not resources_dir - that's also where `make
# install` places icons/Info.plist/etc. before this script runs, and this
# script only ever adds to it (qt_menu.nib, framework Info.plists), never
# removes, so it doesn't have the same stale-content problem.
subprocess.check_call(['rm', '-rf', frameworks_dir])
subprocess.check_call(['rm', '-rf', plugins_dir])
commands.append(['mkdir', '-p', frameworks_dir])
commands.append(['mkdir', '-p', resources_dir])

fixed_libraries = set()
fixed_frameworks = set()


def GetBrokenLibraries(binary):
  output = subprocess.Popen(
      [OTOOL, '-L', binary], stdout=subprocess.PIPE).communicate()[0].decode('utf-8')
  broken_libs = {'frameworks': [], 'libs': []}
  for line in [x.split(' ')[0].lstrip() for x in output.split('\n')[1:]]:
    if not line:  # skip empty lines
      continue
    if os.path.basename(binary) in line:
      continue
    elif re.match(r'^\s*/System/', line):
      continue  # System framework
    elif re.match(r'^\s*/usr/lib/', line):
      continue  # unix style system library
    elif re.match(r'Breakpad', line):
      continue  # Manually added by cmake.
    elif re.match(r'^\s*@loader_path', line):
      abs_path = os.path.join(
        os.path.dirname(binary),
        *os.path.split(line)[1:],
      )
      broken_libs['libs'].append(abs_path)
    elif re.match(r'^\s*@executable_path', line):
      # Potentially already fixed library
      relative_path = os.path.join(*line.split('/')[3:])
      if not os.path.exists(os.path.join(frameworks_dir, relative_path)):
        if re.search(r'\w+\.framework', line):
          broken_libs['frameworks'].append(relative_path)
        else:
          broken_libs['libs'].append(relative_path)
    elif re.search(r'\w+\.framework', line):
      broken_libs['frameworks'].append(line)
    else:
      broken_libs['libs'].append(line)

  return broken_libs


def FindFramework(path):
  for prefix in STRIP_PREFIX:
    if path.startswith(prefix):
      path = path[len(prefix):]
      break
  for search_path in FRAMEWORK_SEARCH_PATH:
    abs_path = os.path.join(search_path, path)
    if os.path.exists(abs_path):
      LOGGER.debug("Found framework '%s' in '%s'", path, search_path)
      return abs_path

  raise CouldNotFindFrameworkError(path)


def GetRPaths(binary):
  output = subprocess.Popen(
      [OTOOL, '-l', binary], stdout=subprocess.PIPE).communicate()[0].decode('utf-8')
  lines = output.split('\n')
  rpaths = []
  for i, line in enumerate(lines):
    if line.strip() == 'cmd LC_RPATH':
      for candidate in lines[i:i + 5]:
        m = re.match(r'\s*path (.*) \(offset \d+\)', candidate)
        if m:
          rpaths.append(m.group(1))
          break
  return rpaths


def ResolveRPath(path, referencing_binary):
  # @rpath entries are resolved against the LC_RPATH commands of the binary
  # that references them - most commonly @loader_path, meaning "the
  # directory containing the referencing binary itself".
  suffix = path[len('@rpath/'):]
  for rpath in GetRPaths(referencing_binary):
    if rpath.startswith('@loader_path'):
      rpath_dir = os.path.normpath(os.path.join(
          os.path.dirname(referencing_binary), rpath[len('@loader_path'):].lstrip('/')))
    elif rpath.startswith('@'):
      continue  # @executable_path etc. isn't meaningful pre-bundling.
    else:
      rpath_dir = rpath
    candidate = os.path.join(rpath_dir, suffix)
    if os.path.exists(candidate):
      return candidate
  return None


def FindLibrary(path, referencing_binary=None):
  if os.path.exists(path):
    return path
  if path.startswith('@rpath/') and referencing_binary:
    resolved = ResolveRPath(path, referencing_binary)
    if resolved:
      LOGGER.debug("Found library '%s' via rpath of '%s'", path,
                    referencing_binary)
      return resolved
  for search_path in LIBRARY_SEARCH_PATH:
    abs_path = os.path.join(search_path, path)
    if os.path.exists(abs_path):
      LOGGER.debug("Found library '%s' in '%s'", path, search_path)
      return abs_path

  raise CouldNotFindFrameworkError(path)


def FixAllLibraries(broken_libs, referencing_binary=None):
  for framework in broken_libs['frameworks']:
    FixFramework(framework)
  for lib in broken_libs['libs']:
    FixLibrary(lib, referencing_binary)


def FixFramework(path):
  abs_path = FindFramework(path)
  # Homebrew paths often reach the same real framework through different
  # symlinked prefixes (e.g. .../opt/qt@5/... vs .../Cellar/qt@5/5.x/...),
  # so dedupe by real path rather than the literal string - otherwise the
  # same framework gets copied twice, and the interleaved copy/symlink
  # commands from the two passes can corrupt each other's output (e.g.
  # produce a self-referential Versions/5/5 symlink).
  real_path = os.path.realpath(abs_path)
  if real_path in fixed_frameworks:
    return
  else:
    fixed_frameworks.add(real_path)
  broken_libs = GetBrokenLibraries(abs_path)
  FixAllLibraries(broken_libs, abs_path)

  new_path = CopyFramework(abs_path)
  id = os.sep.join(new_path.split(os.sep)[3:])
  FixFrameworkId(new_path, id)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, new_path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, new_path)


def FixLibrary(path, referencing_binary=None):
  if FindSystemLibrary(os.path.basename(path)) is not None:
    return
  abs_path = FindLibrary(path, referencing_binary)
  real_path = os.path.realpath(abs_path)
  if real_path in fixed_libraries:
    return
  else:
    fixed_libraries.add(real_path)
  broken_libs = GetBrokenLibraries(abs_path)
  FixAllLibraries(broken_libs, abs_path)

  new_path = CopyLibrary(abs_path)
  FixLibraryId(new_path)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, new_path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, new_path)


def FixPlugin(abs_path, subdir):
  broken_libs = GetBrokenLibraries(abs_path)
  FixAllLibraries(broken_libs, abs_path)

  new_path = CopyPlugin(abs_path, subdir)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, new_path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, new_path)


def FixBinary(path):
  broken_libs = GetBrokenLibraries(path)
  FixAllLibraries(broken_libs, path)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, path)


def CopyLibrary(path):
  new_path = os.path.join(frameworks_dir, os.path.basename(path))
  args = ['cp', path, new_path]
  commands.append(args)
  commands.append(['chmod', '+w', new_path])
  LOGGER.info("Copying library '%s'", path)
  return new_path


def CopyPlugin(path, subdir):
  new_path = os.path.join(plugins_dir, subdir, os.path.basename(path))
  args = ['mkdir', '-p', os.path.dirname(new_path)]
  commands.append(args)
  args = ['cp', path, new_path]
  commands.append(args)
  commands.append(['chmod', '+w', new_path])
  LOGGER.info("Copying plugin '%s'", path)
  return new_path


def CopyFramework(src_binary):
  while os.path.islink(src_binary):
    src_binary = os.path.realpath(src_binary)

  m = re.match(r'(.*/([^/]+)\.framework)/Versions/([^/]+)/.*', src_binary)
  if not m:
    raise CouldNotParseFrameworkNameError(src_binary)

  src_base = m.group(1)
  name = m.group(2)
  version = m.group(3)

  LOGGER.info('Copying framework %s version %s', name, version)

  dest_base = os.path.join(frameworks_dir, '%s.framework' % name)
  dest_dir = os.path.join(dest_base, 'Versions', version)
  dest_binary = os.path.join(dest_dir, name)

  commands.append(['mkdir', '-p', dest_dir])
  commands.append(['cp', src_binary, dest_binary])
  commands.append(['chmod', '+w', dest_binary])

  # Copy special files from various places:
  #   QtCore has Resources/qt_menu.nib (copy to app's Resources)
  #   Sparkle has Resources/*
  #   Qt* have Resources/Info.plist
  #
  # Deliberately os.path.join(src_base, 'Versions', version, 'Resources')
  # rather than os.path.join(src_base, 'Resources'): the latter goes through
  # the framework's top-level "Resources -> Versions/Current/Resources"
  # symlink, which - since dest_dir below is this same framework's own
  # Versions/<version> directory - can make `cp -r` copy part of the
  # framework's own Versions/<version> tree into itself (e.g. a
  # self-referential Versions/A/A or a nested Resources/Resources symlink).
  # Using the already-resolved version dir directly sidesteps that.
  resources_src = os.path.join(src_base, 'Versions', version, 'Resources')
  menu_nib = os.path.join(resources_src, 'qt_menu.nib')
  if os.path.exists(menu_nib):
    LOGGER.info("Copying qt_menu.nib '%s'", menu_nib)
    commands.append(['cp', '-r', menu_nib, resources_dir])
  elif os.path.exists(resources_src):
    LOGGER.info("Copying resources dir '%s'", resources_src)
    commands.append(['cp', '-r', resources_src, dest_dir])

  info_plist = os.path.join(src_base, 'Contents', 'Info.plist')
  if os.path.exists(info_plist):
    LOGGER.info("Copying special file '%s'", info_plist)
    resources_dest = os.path.join(dest_dir, 'Resources')
    commands.append(['mkdir', resources_dest])
    commands.append(['cp', '-r', info_plist, resources_dest])

  # Create symlinks in the Framework to make it look like
  # https://developer.apple.com/library/mac/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/FrameworkAnatomy.html
  commands.append([
      'ln', '-sf', 'Versions/Current/%s' % name, os.path.join(dest_base, name)
  ])
  commands.append([
      'ln', '-sf', 'Versions/Current/Resources',
      os.path.join(dest_base, 'Resources')
  ])
  commands.append(
      ['ln', '-sf', version, os.path.join(dest_base, 'Versions/Current')])

  return dest_binary


def FixId(path, library_name):
  id = '@executable_path/../Frameworks/%s' % library_name
  args = [INSTALL_NAME_TOOL, '-id', id, path]
  commands.append(args)


def FixLibraryId(path):
  library_name = os.path.basename(path)
  FixId(path, library_name)


def FixFrameworkId(path, id):
  FixId(path, id)


def FixInstallPath(library_path, library, new_path):
  args = [INSTALL_NAME_TOOL, '-change', library_path, new_path, library]
  commands.append(args)


def FindSystemLibrary(library_name):
  for path in ['/lib', '/usr/lib']:
    full_path = os.path.join(path, library_name)
    if os.path.exists(full_path):
      return full_path
  return None


def FixLibraryInstallPath(library_path, library):
  system_library = FindSystemLibrary(os.path.basename(library_path))
  if system_library is None:
    new_path = '@executable_path/../Frameworks/%s' % os.path.basename(
        library_path)
    FixInstallPath(library_path, library, new_path)
  else:
    FixInstallPath(library_path, library, system_library)


def FixFrameworkInstallPath(library_path, library):
  parts = library_path.split(os.sep)
  for i, part in enumerate(parts):
    if re.match(r'\w+\.framework', part):
      full_path = os.path.join(*parts[i:])
      break
  new_path = '@executable_path/../Frameworks/%s' % full_path
  FixInstallPath(library_path, library, new_path)


def FindXinePlugin(name):
  for path in XINEPLUGIN_SEARCH_PATH:
    if os.path.exists(path):
      for dir, dirs, files in os.walk(path):
        if name in files:
          return os.path.join(dir, name)
  raise CouldNotFindXinePluginError(name)


def FindQtPlugin(name):
  for path in QT_PLUGINS_SEARCH_PATH:
    if os.path.exists(path):
      if os.path.exists(os.path.join(path, name)):
        return os.path.join(path, name)
  raise CouldNotFindQtPluginError(name)


def FindGstreamerPlugin(name):
  for path in GSTREAMER_SEARCH_PATH:
    if os.path.exists(path):
      for dir, dirs, files in os.walk(path):
        if name in files:
          return os.path.join(dir, name)
  raise CouldNotFindGstreamerPluginError(name)


def FindGioModule(name):
  for path in GIO_MODULES_SEARCH_PATH:
    if os.path.exists(path):
      for dir, dirs, files in os.walk(path):
        if name in files:
          return os.path.join(dir, name)
  raise CouldNotFindGioModuleError(name)


def main():
  logging.basicConfig(
      filename='macdeploy.log',
      level=logging.DEBUG,
      format='%(asctime)s %(levelname)-8s %(message)s')

  FixBinary(binary)

  for plugin in GSTREAMER_PLUGINS:
    FixPlugin(FindGstreamerPlugin(plugin), 'gstreamer')

  FixPlugin(FindGstreamerPlugin('gst-plugin-scanner'), '.')
  FixPlugin(FindGioModule('libgiognutls.so'), 'gio-modules')

  try:
    FixPlugin('clementine-spotifyblob', '.')
    FixPlugin('clementine-tagreader', '.')
  except:
    print('Failed to find blob: %s' % traceback.format_exc())

  for plugin in QT_PLUGINS:
    FixPlugin(FindQtPlugin(plugin), os.path.dirname(plugin))

  if len(sys.argv) <= 2:
    print('Would run %d commands:' % len(commands))
    for command in commands:
      print(' '.join(command))

    print('OK?')
    input()

  for command in commands:
    p = subprocess.Popen(command)
    os.waitpid(p.pid, 0)


if __name__ == "__main__":
  main()
