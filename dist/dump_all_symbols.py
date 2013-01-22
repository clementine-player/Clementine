import argparse
import cStringIO
import errno
import gzip
import json
import logging
import os
import re
import requests
import shutil
import subprocess
import sys
import tempfile
import traceback

LDD_RE = re.compile(r'^\t([^ ]+) => ([^ ]+) ', re.MULTILINE)
OTOOL_RE = re.compile(r'^\t([^ ]+) \(', re.MULTILINE)

DEFAULT_CRASHREPORTING_HOSTNAME = "crashes.clementine-player.org"
DEFAULT_SYMBOLS_DIRECTORY = "symbols"
DEFAULT_DUMP_SYMS_BINARY = "3rdparty/google-breakpad/dump_syms"

SYMBOLUPLOAD_URL = "http://%s/api/upload/symbols"


class BaseDumperImpl(object):
  def GetLinkedLibraries(self, filename):
    raise NotImplementedError

  def DebugSymbolsFilename(self, filename):
    return filename


class LinuxDumperImpl(BaseDumperImpl):
  def GetLinkedLibraries(self, filename):
    stdout = subprocess.check_output(["ldd", filename])
    for match in LDD_RE.finditer(stdout):
      yield os.path.realpath(match.group(2))


class MacDumperImpl(BaseDumperImpl):
  def GetLinkedLibraries(self, filename):
    stdout = subprocess.check_output(["otool", "-L", filename])
    executable_path = os.path.dirname(filename)
    for match in OTOOL_RE.finditer(stdout):
      path = match.group(1)
      path = path.replace("@executable_path", executable_path)
      path = path.replace("@loader_path", executable_path)
      yield path

  def DebugSymbolsFilename(self, filename):
    class Context(object):
      def __init__(self):
        self.temp_dir = tempfile.mkdtemp()
        self.temp_file = os.path.join(self.temp_dir, os.path.basename(filename))

      def __enter__(self):
        logging.info("Reading dwarf symbols from '%s'", filename)

        # Extract the dwarf symbols into the temporary file
        dsymutil = subprocess.Popen(
            ["dsymutil", filename, "-f", "-o", self.temp_file],
            stdout=subprocess.PIPE)
        stdout, _ = dsymutil.communicate()

        if "no debug symbols" in stdout:
          return filename
        else:
          return self.temp_file

      def __exit__(self, exc_type, exc_value, traceback):
        shutil.rmtree(self.temp_dir)

    return Context()


class Dumper(object):
  def __init__(self, symbols_directory, dump_syms_binary):
    self.symbols_directory = symbols_directory
    self.dump_syms_binary = dump_syms_binary

    self.visited_executables = set()
    self.binary_namehashes = []

    if sys.platform == "darwin":
      self.impl = MacDumperImpl()
    else:
      self.impl = LinuxDumperImpl()

  def NormaliseFilename(self, filename):
    return os.path.abspath(filename)

  def SymbolFilename(self, binary_namehash):
    return os.path.join(
        self.symbols_directory, binary_namehash[0], binary_namehash[1],
        binary_namehash[0] + ".sym")

  def Dump(self, binary_filename):
    # Check we haven't processed this file before.
    binary_filename = self.NormaliseFilename(binary_filename)
    if binary_filename in self.visited_executables:
      return
    self.visited_executables.add(binary_filename)

    if not os.path.exists(binary_filename):
      logging.warning("Skipping nonexistent file '%s'", binary_filename)
      return

    # Run dump_syms
    with self.impl.DebugSymbolsFilename(binary_filename) as symbol_filename:
      stdout = subprocess.check_output(
          [self.dump_syms_binary, symbol_filename],
          stderr=subprocess.PIPE)

    # The first line of the output contains the hash.
    first_line = stdout[0:stdout.find("\n")]
    _, os_name, architecture, binary_hash, binary_name = first_line.split(" ")

    # Decide where to write the symbol file.
    binary_namehash = (binary_name, binary_hash)
    symbol_filename = self.SymbolFilename(binary_namehash)
    self.binary_namehashes.append(binary_namehash)
    logging.info("Writing '%s'", symbol_filename)

    # Create parent directories.
    try:
      os.makedirs(os.path.dirname(symbol_filename))
    except OSError, ex:
      # It's fine if the directory already exists.
      if ex.errno != errno.EEXIST:
        raise

    # Output the file.
    with open(symbol_filename, "w") as handle:
      handle.write(stdout)

    # Now do the same for any linked libraries.
    for path in self.impl.GetLinkedLibraries(binary_filename):
      self.Dump(path)


class Uploader(object):
  def __init__(self, crashreporting_hostname, dumper):
    self.crashreporting_hostname = crashreporting_hostname
    self.dumper = dumper

  def Compress(self, filename):
    stringio = cStringIO.StringIO()
    with gzip.GzipFile(fileobj=stringio, mode="w") as gzip_file:
      with open(filename) as original_file:
        shutil.copyfileobj(original_file, gzip_file)

    original_size = os.path.getsize(filename)
    new_size = stringio.tell()
    logging.info("Compressed %s from %d bytes to %d bytes (%02f%%)" % (
        filename, original_size, new_size,
        float(new_size) / original_size * 100))

    stringio.seek(0)
    return stringio

  def UploadMissing(self):
    name_hashes = {"%s/%s" % x: x for x in self.dumper.binary_namehashes}

    # Which symbols aren't on the server yet?
    url = SYMBOLUPLOAD_URL % self.crashreporting_hostname
    response = requests.post(url, data={
      "symbol": name_hashes.keys(),
    }).json()

    for path, url in response["urls"].items():
      try:
        symbol_filename = self.dumper.SymbolFilename(name_hashes[path])
        logging.info("Uploading '%s'", symbol_filename)

        upload_response = requests.put(url,
            data=self.Compress(symbol_filename).read(),
            headers={'content-type': 'application/gzip'})
        upload_response.raise_for_status()
      except Exception:
        logging.warning("Failed to upload file '%s': %s" % (
            symbol_filename, traceback.format_exc()))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("--symbols_directory", default=DEFAULT_SYMBOLS_DIRECTORY)
  parser.add_argument("--dump_syms_binary", default=DEFAULT_DUMP_SYMS_BINARY)
  parser.add_argument("--crashreporting_hostname", default=DEFAULT_CRASHREPORTING_HOSTNAME)
  parser.add_argument("filenames", nargs='+')
  args = parser.parse_args()

  logging.basicConfig(level=logging.DEBUG,
                      format="%(asctime)s %(levelname)-7s %(message)s")
  logging.getLogger("requests").setLevel(logging.WARNING)

  dumper = Dumper(args.symbols_directory, args.dump_syms_binary)
  uploader = Uploader(args.crashreporting_hostname, dumper)

  for filename in args.filenames:
    dumper.Dump(filename)

  uploader.UploadMissing()


if __name__ == "__main__":
  main()
