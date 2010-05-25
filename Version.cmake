# Change this file when releasing a new version.
# You'll also need to add a new changelog entry in:
#   debian/changelog
#   dist/clementine.spec.in

# Version numbers.  For m.n rc, use m.(n-1).99
set(CLEMENTINE_VERSION_MAJOR 0)
set(CLEMENTINE_VERSION_MINOR 3)
set(CLEMENTINE_VERSION_PATCH 2)
set(CLEMENTINE_VERSION_STRING "0.3.2")       # Displayed in GUIs
set(CLEMENTINE_VERSION_SHORT_STRING "0.3.2") # Used in filenames


if (CLEMENTINE_VERSION_PATCH)
  set(CLEMENTINE_VERSION ${CLEMENTINE_VERSION_MAJOR}.${CLEMENTINE_VERSION_MINOR}.${CLEMENTINE_VERSION_PATCH})
else (CLEMENTINE_VERSION_PATCH)
  set(CLEMENTINE_VERSION ${CLEMENTINE_VERSION_MAJOR}.${CLEMENTINE_VERSION_MINOR})
endif (CLEMENTINE_VERSION_PATCH)
