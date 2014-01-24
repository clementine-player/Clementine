import getpass
import os
import re
import subprocess
import sys

import googlecode_upload


PROJECT_NAME = "clementine-player"

FILENAME_PATTERNS = {
  "deb": "clementine_%(major)s.%(minor)s.%(patch)s%(tildeprerelease)s~%(distro)s_%(debarch)s.deb",
  "rpm": "clementine-%(major)s.%(minor)s.%(patch)s-%(rpmrelease)s.%(distro)s.%(rpmarch)s.rpm",
  "exe": "ClementineSetup-%(major)s.%(minor)s.%(patch)s%(prerelease)s.exe",
  "dmg": "clementine-%(major)s.%(minor)s.%(patch)s%(prerelease)s.dmg",
  "tar.gz": "clementine-%(major)s.%(minor)s.%(patch)s%(prerelease)s.tar.gz",
}

LABELS = {
  "deb":      ["Type-Package", "OpSys-Linux"],
  "rpm":      ["Type-Package", "OpSys-Linux", "Distro-Fedora"],
  "exe":      ["Type-Package", "OpSys-Windows", "Arch-i386"],
  "dmg":      ["Type-Package", "OpSys-OSX", "Distro-Lion", "Arch-x86-64"],
  32:         ["Arch-i386"],
  64:         ["Arch-x86-64"],
  "lucid":    ["Distro-Ubuntu"],
  "precise":  ["Distro-Ubuntu"],
  "quantal":  ["Distro-Ubuntu"],
  "raring":   ["Distro-Ubuntu"],
  "saucy":    ["Distro-Ubuntu"],
  "squeeze":  ["Distro-Debian"],
  "wheezy":   ["Distro-Debian"],
}


MIN_SIZE = {
  "deb": 5000000,
  "rpm": 4000000,
  "exe": 18000000,
  "dmg": 24000000,
  "tar.gz": 8000000,
}

DEB_ARCH = {
  32: "i386",
  64: "amd64",
}

RPM_ARCH = {
  32: "i686",
  64: "x86_64",
}

DESCRIPTIONS = {
  ("deb", "lucid"):     "for Ubuntu Lucid (10.04)",
  ("deb", "precise"):   "for Ubuntu Precise (12.04)",
  ("deb", "quantal"):   "for Ubuntu Quantal (12.10)",
  ("deb", "raring"):    "for Ubuntu Raring (13.04)",
  ("deb", "saucy"):     "for Ubuntu Saucy (13.10)",
  ("deb", "squeeze"):   "for Debian Squeeze",
  ("deb", "wheezy"):    "for Debian Wheezy",
  ("rpm", "fc18"):      "for Fedora 18",
  ("rpm", "fc19"):      "for Fedora 19",
  ("exe", None):        "for Windows",
  ("dmg", None):        "for Mac OS X",
  ("tar.gz", None):     "source",
}

RELEASES = [
  ("deb", "lucid", 32),
  ("deb", "lucid", 64),
  ("deb", "precise", 32),
  ("deb", "precise", 64),
  ("deb", "quantal", 32),
  ("deb", "quantal", 64),
  ("deb", "raring", 32),
  ("deb", "raring", 64),
  ("deb", "saucy", 32),
  ("deb", "saucy", 64),
  ("deb", "squeeze", 32),
  ("deb", "squeeze", 64),
  ("deb", "wheezy", 32),
  ("deb", "wheezy", 64),
  ("rpm", "fc18", 32),
  ("rpm", "fc18", 64),
  ("rpm", "fc19", 32),
  ("rpm", "fc19", 64),
  ("exe", None, None),
  ("dmg", None, None),
  ("tar.gz", None, None),
]


class VersionInfo(object):
  def __init__(self, root_dir):
    filename = os.path.join(root_dir, "cmake/Version.cmake")
    data = open(filename).read()

    self.info = {
      "major":      self._version(data, "MAJOR"),
      "minor":      self._version(data, "MINOR"),
      "patch":      self._version(data, "PATCH"),
      "prerelease": self._version(data, "PRERELEASE"),
    }

    for key, value in self.info.items():
      setattr(self, key, value)

  def _version(self, data, part):
    regex = r"^set\(CLEMENTINE_VERSION_%s (\w+)\)$" % part
    match = re.search(regex, data, re.MULTILINE)

    if not match:
      return ""

    return match.group(1)

  def filename(self, release):
    (package, distro, arch) = release

    data = dict(self.info)
    data["distro"]          = distro
    data["rpmarch"]         = RPM_ARCH.get(arch, None)
    data["debarch"]         = DEB_ARCH.get(arch, None)
    data["tildeprerelease"] = ""
    data["rpmrelease"]      = "1"

    if data["prerelease"]:
      data["tildeprerelease"] = "~%s"  % data["prerelease"]
      data["rpmrelease"]      = "0.%s" % data["prerelease"]

    return FILENAME_PATTERNS[package] % data

  def description(self, release):
    (package, distro, arch) = release

    version_name = "%(major)s.%(minor)s" % self.info

    if self.patch is not "0":
      version_name += ".%s" % self.patch

    if self.prerelease:
      version_name += " %s" % self.prerelease.upper()

    os_name = DESCRIPTIONS[(package, distro)]

    if arch is not None:
      os_name += " %d-bit" % arch

    return "Clementine %s %s" % (version_name, os_name)

  def labels(self, release):
    (package, distro, arch) = release

    labels = LABELS.get(package, []) + \
             LABELS.get(distro, []) + \
             LABELS.get(arch, [])

    if self.prerelease.startswith("rc"):
      labels.append("Release-RC")
    elif self.prerelease.startswith("beta"):
      labels.append("Release-Beta")
    else:
      labels.append("Release-Stable")

    return labels


def get_google_code_password(username):
  # Try to read it from the .netrc first
  NETRC_REGEX = re.compile(
      r'^machine\s+code\.google\.com\s+'
      r'login\s+([^@]+)@[^\s]+\s+'
      r'password\s+([^\s+])')

  try:
    for line in open(os.path.expanduser("~/.netrc")):
      match = NETRC_REGEX.match(line)
      if match and match.group(1) == username:
        print "Using password from ~/.netrc"
        return match.group(2)
  except IOError:
    pass

  # Prompt the user
  password = getpass.getpass("Google Code password (different to your Google account): ")
  if not password:
    return None

  return password


def main():
  dist_dir = os.path.dirname(os.path.abspath(__file__))
  root_dir = os.path.normpath(os.path.join(dist_dir, ".."))

  # Read the version file
  version = VersionInfo(root_dir)

  # Display the files that will be uploaded
  for release in RELEASES:
    filename = version.filename(release)
    description = version.description(release)

    if not os.path.exists(filename):
      print
      print "%s - file not found" % filename
      print "Run this script from a directory containing all the release packages"
      return 1

    size = os.path.getsize(filename)

    if size < MIN_SIZE[release[0]]:
      print
      print "%s - file not big enough" % filename
      print "%s files are expected to be at least %d bytes, but this was %d bytes" % (
        release[0], MIN_SIZE[release[0]], size)
      return 1

    labels = version.labels(release)

    print "%-40s %15s   %-55s %s" % (filename, "%d bytes" % size, description, " ".join(sorted(labels)))

  print

  # Prompt for username and password
  username = raw_input("Google username: ")
  if not username:
    return 1

  password = get_google_code_password(username)
  if password is None:
    return 1

  print

  # Upload everything
  for release in RELEASES:
    (status, reason, url) = googlecode_upload.upload(
      file=version.filename(release),
      project_name=PROJECT_NAME,
      user_name=username,
      password=password,
      summary=version.description(release),
      labels=version.labels(release),
    )

    if status != 201:
      print "%s: (%d) %s" % (version.filename(release), status, reason)
    else:
      print "Uploaded %s" % url

  return 0


if __name__ == "__main__":
  sys.exit(main())
