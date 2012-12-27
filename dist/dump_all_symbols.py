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

LDD_RE = re.compile(r'^\t([^ ]+) => ([^ ]+) ', re.MULTILINE)

DEFAULT_CRASHREPORTING_HOSTNAME = "crashes.clementine-player.org"
DEFAULT_SYMBOLS_DIRECTORY = "symbols"
DEFAULT_DUMP_SYMS_BINARY = "3rdparty/google-breakpad/dump_syms"

UPLOAD_URL = "http://%s/upload/symbols"
SYMBOLCHECK_URL = "http://%s/symbolcheck"


class Dumper(object):
  def __init__(self, symbols_directory, dump_syms_binary):
    self.symbols_directory = symbols_directory
    self.dump_syms_binary = dump_syms_binary

    self.visited_executables = set()
    self.binary_namehashes = []

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

    # Run dump_syms
    stdout = subprocess.check_output(
        [self.dump_syms_binary, binary_filename],
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
    for path in self.GetLinkedLibraries(binary_filename):
      self.Dump(path)

  def GetLinkedLibraries(self, filename):
    stdout = subprocess.check_output(["ldd", filename])
    for match in LDD_RE.finditer(stdout):
      yield os.path.realpath(match.group(2))


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
    # Which symbols aren't on the server yet?
    url = SYMBOLCHECK_URL % self.crashreporting_hostname
    missing_symbols = requests.post(url, data=json.dumps({
      "binary_info": [
        {
          "name": x[0],
          "hash": x[1],
        } for x in self.dumper.binary_namehashes
      ],
    })).json()

    for info in missing_symbols["missing_symbols"]:
      symbol_filename = self.dumper.SymbolFilename((info["name"], info["hash"]))
      logging.info("Uploading '%s'", symbol_filename)

      # Get the upload URL
      url = UPLOAD_URL % self.crashreporting_hostname
      redirect_response = requests.get(url, allow_redirects=False)
      blobstore_url = redirect_response.headers["Location"]

      # Upload the symbols
      requests.post(blobstore_url,
          files={"data": (symbol_filename + ".gz",
                          self.Compress(symbol_filename))},
          data={
            "name": info["name"],
            "hash": info["hash"],
          }
      )


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("--symbols_directory", default=DEFAULT_SYMBOLS_DIRECTORY)
  parser.add_argument("--dump_syms_binary", default=DEFAULT_DUMP_SYMS_BINARY)
  parser.add_argument("--crashreporting_hostname", default=DEFAULT_CRASHREPORTING_HOSTNAME)
  parser.add_argument("filenames", nargs='+')
  args = parser.parse_args()

  logging.basicConfig(level=logging.DEBUG,
                      format="%(asctime)s  %(levelname)s  %(message)s")

  dumper = Dumper(args.symbols_directory, args.dump_syms_binary)
  uploader = Uploader(args.crashreporting_hostname, dumper)

  for filename in args.filenames:
    dumper.Dump(filename)

  uploader.UploadMissing()


if __name__ == "__main__":
  main()
