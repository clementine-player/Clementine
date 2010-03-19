#!/usr/bin/python

import os
import re
import subprocess
import sys

FRAMEWORK_SEARCH_PATH=['/Library/Frameworks', os.path.join(os.environ['HOME'], 'Library/Frameworks')]
LIBRARY_SEARCH_PATH=['/usr/local/lib', '/sw/lib']

XINE_PLUGINS = [
    'xineplug_ao_out_coreaudio.so',
    'xineplug_decode_a52.so',
    'xineplug_decode_dts.so',
    'xineplug_decode_faad.so',
    'xineplug_decode_mad.so',
    'xineplug_decode_mpc.so',
    'xineplug_decode_vorbis.so',
    'xineplug_dmx_audio.so',
    'xineplug_dmx_ogg.so',
    'xineplug_flac.so',
    'xineplug_inp_cdda.so',
    'xineplug_inp_file.so',
    'xineplug_inp_http.so',
    'xineplug_inp_net.so',
    'xineplug_inp_rtp.so',
    'xineplug_inp_rtsp.so',
    'xineplug_post_audio_filters.so',
]
XINEPLUGIN_SEARCH_PATH=[
    '/usr/local/lib/xine/plugins',
    '/usr/lib/xine/plugins',
    '/sw/lib/xine/plugins',
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
    'imageformats/libqtiff.dylib',
    'sqldrivers/libqsqlite.dylib',
]
QT_PLUGINS_SEARCH_PATH=[
    '/Developer/Applications/Qt/plugins',
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


if len(sys.argv) < 2:
  print 'Usage: %s <bundle.app>' % sys.argv[0]

bundle_dir = sys.argv[1]

bundle_name = bundle_dir.split('.')[0]

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
    if re.match(r'^\s*/System/', line):
      continue  # System framework
    elif re.match(r'^\s*/usr/lib/', line):
      continue  # unix style system library
    elif re.match(r'^\s*\w+\.framework', line):
      broken_libs['frameworks'].append(line)
    elif re.match(r'^\s*@executable_path', line):
      # Potentially already fixed library
      relative_path = os.path.join(*line.split('/')[3:])
      if not os.path.exists(os.path.join(frameworks_dir, relative_path)):
        broken_libs['frameworks'].append(relative_path)
    else:
      broken_libs['libs'].append(line)

  return broken_libs

def FindFramework(path):
  for search_path in FRAMEWORK_SEARCH_PATH:
    abs_path = os.path.join(search_path, path)
    if os.path.exists(abs_path):
      return abs_path

  raise CouldNotFindFrameworkError(path)

def FindLibrary(path):
  if os.path.exists(path):
    return path
  for search_path in LIBRARY_SEARCH_PATH:
    abs_path = os.path.join(search_path, path)
    if os.path.exists(abs_path):
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
  FixFrameworkId(new_path, path)
  for framework in broken_libs['frameworks']:
    FixFrameworkInstallPath(framework, new_path)
  for library in broken_libs['libs']:
    FixLibraryInstallPath(library, new_path)

def FixLibrary(path):
  if path in fixed_libraries:
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
  args = ['cp', '-f', path, new_path]
  commands.append(args)
  return new_path

def CopyPlugin(path, subdir):
  new_path = os.path.join(plugins_dir, subdir, os.path.basename(path))
  args = ['mkdir', '-p', os.path.dirname(new_path)]
  commands.append(args)
  args = ['cp', '-f', path, new_path]
  commands.append(args)
  return new_path

def CopyFramework(path):
  parts = path.split(os.sep)
  for i, part in enumerate(parts):
    if re.match(r'\w+\.framework', part):
      full_path = os.path.join(frameworks_dir, *parts[i:-1])
      break
  args = ['mkdir', '-p', full_path]
  commands.append(args)
  args = ['cp', '-f', path, full_path]
  commands.append(args)
  return os.path.join(frameworks_dir, path)

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

def FixLibraryInstallPath(library_path, library):
  new_path = '@executable_path/../Frameworks/%s' % os.path.basename(library_path)
  FixInstallPath(library_path, library, new_path)

def FixFrameworkInstallPath(library_path, library):
  new_path = '@executable_path/../Frameworks/%s' % library_path
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


FixBinary(binary)

for plugin in XINE_PLUGINS:
  FixPlugin(FindXinePlugin(plugin), 'xine')

for plugin in QT_PLUGINS:
  FixPlugin(FindQtPlugin(plugin), os.path.dirname(plugin))

print 'Would run %d commands:' % len(commands)
for command in commands:
  print ' '.join(command)

print 'OK?'
raw_input()

for command in commands:
  p = subprocess.Popen(command)
  os.waitpid(p.pid, 0)

qtconf = open(os.path.join(resources_dir, 'qt.conf'), 'w')
qtconf.write('''[Paths]
Plugins = PlugIns
''')
qtconf.close()
