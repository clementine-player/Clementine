#!/usr/bin/python

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

from distutils import spawn
import logging
import os
import re
import subprocess
import sys
import traceback

LOGGER = logging.getLogger('macdeploy')

FRAMEWORK_SEARCH_PATH = [
    '/target', '/target/lib', '/Library/Frameworks',
    os.path.join(os.environ['HOME'], 'Library/Frameworks')
]

LIBRARY_SEARCH_PATH = ['/target', '/target/lib', '/usr/local/lib', '/sw/lib']

GSTREAMER_PLUGINS = [
    # Core plugins
    'libgstapp.so',
    'libgstaudioconvert.so',
    'libgstaudiofx.so',
    'libgstaudiotestsrc.so',
    'libgstaudioresample.so',
    'libgstautodetect.so',
    'libgstcoreelements.so',
    'libgstequalizer.so',
    'libgstgdp.so',
    'libgstosxaudio.so',
    'libgstplayback.so',
    'libgsttcp.so',
    'libgsttypefindfunctions.so',
    'libgstudp.so',
    'libgstvolume.so',

    # Codecs
    'libgstapetag.so',
    'libgstasf.so',
    'libgstaudioparsers.so',
    'libgstfaac.so',
    'libgstfaad.so',
    'libgstflac.so',
    'libgstid3demux.so',
    'libgstisomp4.so',
    'libgstlame.so',
    'libgstlibav.so',
    'libgstmad.so',
    'libgstmms.so',
    # TODO: Bring back Musepack support.
    'libgstogg.so',
    'libgstopus.so',
    'libgstreplaygain.so',
    'libgstspeex.so',
    'libgsttaglib.so',
    'libgstvorbis.so',
    'libgstwavpack.so',
    'libgstwavparse.so',

    # HTTP src support
    'libgstsouphttpsrc.so',

    # Icecast support
    'libgsticydemux.so',

    # CD support
    'libgstcdio.so',

    # RTSP streaming
    'libgstrtp.so',
    'libgstrtsp.so',
]

GSTREAMER_SEARCH_PATH = [
    '/target/lib/gstreamer-1.0',
    '/target/libexec/gstreamer-1.0',
]

QT_PLUGINS = [
    'accessible/libqtaccessiblewidgets.dylib',
    'codecs/libqcncodecs.dylib',
    'codecs/libqjpcodecs.dylib',
    'codecs/libqkrcodecs.dylib',
    'codecs/libqtwcodecs.dylib',
    'iconengines/libqsvgicon.dylib',
    'imageformats/libqgif.dylib',
    'imageformats/libqico.dylib',
    'imageformats/libqjpeg.dylib',
    'imageformats/libqmng.dylib',
    'imageformats/libqsvg.dylib',
]
QT_PLUGINS_SEARCH_PATH = [
    '/target/plugins',
    '/usr/local/Trolltech/Qt-4.7.0/plugins',
    '/Developer/Applications/Qt/plugins',
]

GIO_MODULES_SEARCH_PATH = ['/target/lib/gio/modules',]

INSTALL_NAME_TOOL_APPLE = 'install_name_tool'
INSTALL_NAME_TOOL_CROSS = 'x86_64-apple-darwin-%s' % INSTALL_NAME_TOOL_APPLE
INSTALL_NAME_TOOL = INSTALL_NAME_TOOL_CROSS if spawn.find_executable(
    INSTALL_NAME_TOOL_CROSS) else INSTALL_NAME_TOOL_APPLE

OTOOL_APPLE = 'otool'
OTOOL_CROSS = 'x86_64-apple-darwin-%s' % OTOOL_APPLE
OTOOL = OTOOL_CROSS if spawn.find_executable(OTOOL_CROSS) else OTOOL_APPLE


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
  print 'Usage: %s <bundle.app>' % sys.argv[0]

bundle_dir = sys.argv[1]

bundle_name = os.path.basename(bundle_dir).split('.')[0]

commands = []

frameworks_dir = os.path.join(bundle_dir, 'Contents', 'Frameworks')
commands.append(['mkdir', '-p', frameworks_dir])
resources_dir = os.path.join(bundle_dir, 'Contents', 'Resources')
commands.append(['mkdir', '-p', resources_dir])
plugins_dir = os.path.join(bundle_dir, 'Contents', 'PlugIns')
binary = os.path.join(bundle_dir, 'Contents', 'MacOS', bundle_name)

fixed_libraries = set()
fixed_frameworks = set()


def GetBrokenLibraries(binary):
  output = subprocess.Popen(
      [OTOOL, '-L', binary], stdout=subprocess.PIPE).communicate()[0]
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
    elif re.match(r'^\s*@executable_path', line) or re.match(
        r'^\s*@loader_path', line):
      # Potentially already fixed library
      relative_path = os.path.join(*line.split('/')[3:])
      if not os.path.exists(os.path.join(frameworks_dir, relative_path)):
        broken_libs['frameworks'].append(relative_path)
    elif re.search(r'\w+\.framework', line):
      broken_libs['frameworks'].append(line)
    else:
      broken_libs['libs'].append(line)

  return broken_libs


def FindFramework(path):
  for search_path in FRAMEWORK_SEARCH_PATH:
    abs_path = os.path.join(search_path, path)
    if os.path.exists(abs_path):
      LOGGER.debug("Found framework '%s' in '%s'", path, search_path)
      return abs_path

  raise CouldNotFindFrameworkError(path)


def FindLibrary(path):
  if os.path.exists(path):
    return path
  for search_path in LIBRARY_SEARCH_PATH:
    abs_path = os.path.join(search_path, path)
    if os.path.exists(abs_path):
      LOGGER.debug("Found library '%s' in '%s'", path, search_path)
      return abs_path

  raise CouldNotFindFrameworkError(path)


def FixAllLibraries(broken_libs):
  for framework in broken_libs['frameworks']:
    FixFramework(framework)
  for lib in broken_libs['libs']:
    FixLibrary(lib)


def FixFramework(path):
  if path in fixed_frameworks:
    return
  else:
    fixed_frameworks.add(path)
  abs_path = FindFramework(path)
  broken_libs = GetBrokenLibraries(abs_path)
  FixAllLibraries(broken_libs)

  new_path = CopyFramework(abs_path)
  id = os.sep.join(new_path.split(os.sep)[3:])
  FixFrameworkId(new_path, id)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, new_path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, new_path)


def FixLibrary(path):
  if path in fixed_libraries or FindSystemLibrary(os.path.basename(
      path)) is not None:
    return
  else:
    fixed_libraries.add(path)
  abs_path = FindLibrary(path)
  broken_libs = GetBrokenLibraries(abs_path)
  FixAllLibraries(broken_libs)

  new_path = CopyLibrary(abs_path)
  FixLibraryId(new_path)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, new_path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, new_path)


def FixPlugin(abs_path, subdir):
  broken_libs = GetBrokenLibraries(abs_path)
  FixAllLibraries(broken_libs)

  new_path = CopyPlugin(abs_path, subdir)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, new_path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, new_path)


def FixBinary(path):
  broken_libs = GetBrokenLibraries(path)
  FixAllLibraries(broken_libs)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, path)


def CopyLibrary(path):
  new_path = os.path.join(frameworks_dir, os.path.basename(path))
  args = ['cp', path, new_path]
  commands.append(args)
  LOGGER.info("Copying library '%s'", path)
  return new_path


def CopyPlugin(path, subdir):
  new_path = os.path.join(plugins_dir, subdir, os.path.basename(path))
  args = ['mkdir', '-p', os.path.dirname(new_path)]
  commands.append(args)
  args = ['cp', path, new_path]
  commands.append(args)
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

  # Copy special files from various places:
  #   QtCore has Resources/qt_menu.nib (copy to app's Resources)
  #   Sparkle has Resources/*
  #   Qt* have Resources/Info.plist
  resources_src = os.path.join(src_base, 'Resources')
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
  FixPlugin(FindGioModule('libgiolibproxy.so'), 'gio-modules')

  try:
    FixPlugin('clementine-spotifyblob', '.')
    FixPlugin('clementine-tagreader', '.')
  except:
    print 'Failed to find blob: %s' % traceback.format_exc()

  for plugin in QT_PLUGINS:
    FixPlugin(FindQtPlugin(plugin), os.path.dirname(plugin))

  if len(sys.argv) <= 2:
    print 'Would run %d commands:' % len(commands)
    for command in commands:
      print ' '.join(command)

    print 'OK?'
    raw_input()

  for command in commands:
    p = subprocess.Popen(command)
    os.waitpid(p.pid, 0)


if __name__ == "__main__":
  main()
