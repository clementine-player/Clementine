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

import logging
import os
import re
import subprocess
import sys
import traceback

LOGGER = logging.getLogger("macdeploy")

FRAMEWORK_SEARCH_PATH=[
    '/target',
    '/target/lib',
    '/Library/Frameworks',
    os.path.join(os.environ['HOME'], 'Library/Frameworks')
]

LIBRARY_SEARCH_PATH=['/target/lib', '/usr/local/lib', '/sw/lib']


GSTREAMER_PLUGINS=[
    # Core plugins
    'libgstapp.so',
    'libgstaudioconvert.so',
    'libgstaudiofx.so',
    'libgstaudiotestsrc.so',
    'libgstaudioresample.so',
    'libgstautodetect.so',
    'libgstcoreelements.so',
    'libgstdecodebin2.so',
    'libgstequalizer.so',
    'libgstgdp.so',
    'libgstosxaudio.so',
    'libgsttcp.so',
    'libgsttypefindfunctions.so',
    'libgstudp.so',
    'libgstvolume.so',

    # Codecs
    'libgstapetag.so',
    'libgstasf.so',
    'libgstfaac.so',
    'libgstfaad.so',
    'libgstffmpeg.so',
    'libgstflac.so',
    'libgstid3demux.so',
    'libgstlame.so',
    'libgstmad.so',
    'libgstmms.so',
    'libgstmpegaudioparse.so',
    'libgstmusepack.so',
    'libgstogg.so',
    'libgstopus.so',
    'libgstqtdemux.so',
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

GSTREAMER_SEARCH_PATH=[
    '/target/lib/gstreamer-0.10',
    '/target/libexec/gstreamer-0.10',
    '/sw/lib/gstreamer-0.10',
    '/usr/local/lib/gstreamer-0.10',
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
QT_PLUGINS_SEARCH_PATH=[
    '/target/plugins',
    '/usr/local/Trolltech/Qt-4.7.0/plugins',
    '/Developer/Applications/Qt/plugins',
]

GIO_MODULES_SEARCH_PATH=[
    '/target/lib/gio/modules',
]


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

fixed_libraries = []
fixed_frameworks = []

def GetBrokenLibraries(binary):
  output = subprocess.Popen(['otool', '-L', binary], stdout=subprocess.PIPE).communicate()[0]
  broken_libs = {
      'frameworks': [],
      'libs': []}
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
    elif re.match(r'^\s*@executable_path', line) or re.match(r'^\s*@loader_path', line):
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
  if path in fixed_libraries:
    return
  else:
    fixed_libraries.append(path)
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
  if path in fixed_libraries or FindSystemLibrary(os.path.basename(path)) is not None:
    return
  else:
    fixed_libraries.append(path)
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
  args = ['ditto', '--arch=x86_64', path, new_path]
  commands.append(args)
  LOGGER.info("Copying library '%s'", path)
  return new_path

def CopyPlugin(path, subdir):
  new_path = os.path.join(plugins_dir, subdir, os.path.basename(path))
  args = ['mkdir', '-p', os.path.dirname(new_path)]
  commands.append(args)
  args = ['ditto', '--arch=x86_64', path, new_path]
  commands.append(args)
  LOGGER.info("Copying plugin '%s'", path)
  return new_path

def CopyFramework(path):
  parts = path.split(os.sep)
  for i, part in enumerate(parts):
    if re.match(r'\w+\.framework', part):
      full_path = os.path.join(frameworks_dir, *parts[i:-1])
      break
  args = ['mkdir', '-p', full_path]
  commands.append(args)
  args = ['ditto', '--arch=x86_64', path, full_path]
  commands.append(args)

  menu_nib = os.path.join(os.path.split(path)[0], 'Resources', 'qt_menu.nib')
  if os.path.exists(menu_nib):
    args = ['cp', '-r', menu_nib, resources_dir]
    commands.append(args)

  LOGGER.info("Copying framework '%s'", path)
  return os.path.join(full_path, parts[-1])

def FixId(path, library_name):
  id = '@executable_path/../Frameworks/%s' % library_name
  args = ['install_name_tool', '-id', id, path]
  commands.append(args)

def FixLibraryId(path):
  library_name = os.path.basename(path)
  FixId(path, library_name)

def FixFrameworkId(path, id):
  FixId(path, id)

def FixInstallPath(library_path, library, new_path):
  args = ['install_name_tool', '-change', library_path, new_path, library]
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
    new_path = '@executable_path/../Frameworks/%s' % os.path.basename(library_path)
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
  logging.basicConfig(filename="macdeploy.log", level=logging.DEBUG,
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
