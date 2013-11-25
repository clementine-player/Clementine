# Change this file when releasing a new version.

# Version numbers.
set(CLEMENTINE_VERSION_MAJOR 1)
set(CLEMENTINE_VERSION_MINOR 2)
set(CLEMENTINE_VERSION_PATCH 1)
# set(CLEMENTINE_VERSION_PRERELEASE rc4)

# This should be set to OFF in a release branch
set(INCLUDE_GIT_REVISION ON)

# Rules about version number comparison on different platforms:
#   Debian:
#     Two stages are repeated until there are no more characters to compare:
#     one block of consecutive digits (\d+) is compared numerically, then one
#     block of consecutive NON-digits (\D+) is compared lexigraphically,
#     with the exception that ~ sorts before everything else.
#
#     The "upstream version" and "debian revision" are separated by the last
#     dash in the version number.
#
#     Algorithm is in "man deb-version", test comparisons with
#     dpkg --compare-versions.
#
#     These are in sorted order:
#       1.0~rc1
#       1.0~rc2
#       1.0
#       1.0-1-g044287b
#       1.0-506-g044287b
#       1.0.1
#       1.0.2
#       1.0.a
#
#   Rpm:
#     The string is split on non-alphanumeric characters.  Numeric sections are
#     compared numerically and non-numeric sections are compared lexigraphically.
#     If one sections is numeric and the other sections is non-numeric, the
#     numeric sections is always NEWER.
#
#     The "version" and "release" fields are compared with the same algorithm -
#     if the versions are equal the releases are compared to determine which
#     package is newer.
#
#     Algorithm is described in:
#       http://fedoraproject.org/wiki/Packaging:NamingGuidelines#Package_Versioning
#     Test comparisons with:
#       import rpm
#       rpm.labelCompare((epoch, version, release), (epoch, version, release))
#
#     These are in sorted order:
#       1.0-0.rc1
#       1.0-0.rc2
#       1.0-1
#       1.0-2.506-g044287b
#       1.0.1-1
#       1.0.2-1
#
#   Sparkle (mac) and QtSparkle (windows):
#     The strings are split into sections of characters that are all of the same
#     "type" - where a "type" is period, digit, or other.  Sections are then
#     compared against each other - digits are compared numerically and other
#     are compared lexigraphically.  When two sections are of different types,
#     the numeric section is always NEWER.
#
#     If the common parts of both strings are equal, but one string has more
#     sections, the type of the first extra section is used to determine which
#     version is newer.
#     If the extra section is a string, the shorter result is NEWER, otherwise
#     the shorter section is OLDER.  That means that 1.0 is NEWER than 1.0rc1,
#     but 1.0 is OLDER than 1.0.1.
#
#     See compareversions.cpp in QtSparkle.

# Version numbers in Clementine:
#   Deb:
#     With git:    $tagname-$commitcount-g$sha1
#     Without git: $major.$minor.$patch[~$prerelease]
#
#   Rpm:           Version                 Release
#     Prerelease:  $major.$minor.$patch    0.$prerelease
#     Without git: $major.$minor.$patch    1
#     With git:    $tagname                2.$commitcount.g$sha1
#
#   QtSparkle (Windows):
#     With git:    $tagname-$commitcount-g$sha1
#     Without git: $major.$minor.$patch[$prerelease]
#
#   Mac info.plist: CFBundleVersion
#     Prerelease:   4096.$major.$minor.$patch.0
#     Without git:  4096.$major.$minor.$patch.1
#     With git:     4096.$tagname.2.$commitcount
#   The 4096. prefix is because the previous versioning scheme used svn revision
#   numbers, which got up to 3000+.


set(majorminorpatch "${CLEMENTINE_VERSION_MAJOR}.${CLEMENTINE_VERSION_MINOR}.${CLEMENTINE_VERSION_PATCH}")

set(CLEMENTINE_VERSION_DISPLAY "${majorminorpatch}")
set(CLEMENTINE_VERSION_DEB     "${majorminorpatch}")
set(CLEMENTINE_VERSION_RPM_V   "${majorminorpatch}")
set(CLEMENTINE_VERSION_RPM_R   "1")
set(CLEMENTINE_VERSION_SPARKLE "${majorminorpatch}")
set(CLEMENTINE_VERSION_PLIST   "4096.${majorminorpatch}")

if(${CLEMENTINE_VERSION_PATCH} EQUAL "0")
  set(CLEMENTINE_VERSION_DISPLAY "${CLEMENTINE_VERSION_MAJOR}.${CLEMENTINE_VERSION_MINOR}")
endif(${CLEMENTINE_VERSION_PATCH} EQUAL "0")

# Add prerelease
if(CLEMENTINE_VERSION_PRERELEASE)
  set(CLEMENTINE_VERSION_DISPLAY "${CLEMENTINE_VERSION_DISPLAY} ${CLEMENTINE_VERSION_PRERELEASE}")
  set(CLEMENTINE_VERSION_DEB     "${CLEMENTINE_VERSION_DEB}~${CLEMENTINE_VERSION_PRERELEASE}")
  set(CLEMENTINE_VERSION_RPM_R   "0.${CLEMENTINE_VERSION_PRERELEASE}")
  set(CLEMENTINE_VERSION_SPARKLE "${CLEMENTINE_VERSION_SPARKLE}${CLEMENTINE_VERSION_PRERELEASE}")
  set(CLEMENTINE_VERSION_PLIST   "${CLEMENTINE_VERSION_PLIST}.0")
else(CLEMENTINE_VERSION_PRERELEASE)
  set(CLEMENTINE_VERSION_PLIST   "${CLEMENTINE_VERSION_PLIST}.1")
endif(CLEMENTINE_VERSION_PRERELEASE)

# Add git revision
if(FORCE_GIT_REVISION)
  set(GIT_REV ${FORCE_GIT_REVISION})
  set(GIT_INFO_RESULT 0)
else(FORCE_GIT_REVISION)
  find_program(GIT_EXECUTABLE git)

  if(NOT GIT_EXECUTABLE-NOTFOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe
        RESULT_VARIABLE GIT_INFO_RESULT
        OUTPUT_VARIABLE GIT_REV
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
endif()

if(${GIT_INFO_RESULT} EQUAL 0)
  string(REGEX REPLACE "^(.+)-([0-9]+)-(g[a-f0-9]+)$" "\\1;\\2;\\3"
         GIT_PARTS ${GIT_REV})

  if(NOT GIT_PARTS)
    message(FATAL_ERROR "Failed to parse git revision string '${GIT_REV}'")
  endif(NOT GIT_PARTS)

  list(LENGTH GIT_PARTS GIT_PARTS_LENGTH)
  if(GIT_PARTS_LENGTH EQUAL 3)
    list(GET GIT_PARTS 0 GIT_TAGNAME)
    list(GET GIT_PARTS 1 GIT_COMMITCOUNT)
    list(GET GIT_PARTS 2 GIT_SHA1)
    set(HAS_GET_REVISION ON)
  endif(GIT_PARTS_LENGTH EQUAL 3)
endif(${GIT_INFO_RESULT} EQUAL 0)

if(INCLUDE_GIT_REVISION AND HAS_GET_REVISION)
  set(CLEMENTINE_VERSION_DISPLAY "${GIT_REV}")
  set(CLEMENTINE_VERSION_DEB     "${GIT_REV}")
  set(CLEMENTINE_VERSION_RPM_V   "${GIT_TAGNAME}")
  set(CLEMENTINE_VERSION_RPM_R   "2.${GIT_COMMITCOUNT}.${GIT_SHA1}")
  set(CLEMENTINE_VERSION_SPARKLE "${GIT_REV}")
  set(CLEMENTINE_VERSION_PLIST   "4096.${GIT_TAGNAME}.2.${GIT_COMMITCOUNT}")
endif(INCLUDE_GIT_REVISION AND HAS_GET_REVISION)

if(0)
  message(STATUS "Display:  ${CLEMENTINE_VERSION_DISPLAY}")
  message(STATUS "Deb:      ${CLEMENTINE_VERSION_DEB}")
  message(STATUS "Rpm:      ${CLEMENTINE_VERSION_RPM_V}-${CLEMENTINE_VERSION_RPM_R}")
  message(STATUS "Sparkle:  ${CLEMENTINE_VERSION_SPARKLE}")
  message(STATUS "Plist:    ${CLEMENTINE_VERSION_PLIST}")
endif(0)
