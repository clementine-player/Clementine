import clementine
import epydoc.cli
import sys

sys.argv = [
  "epydoc",
  "--html",
  "-o", "output",
  "--introspect-only",
  "-v",
  "--name", "clementine",
  "--url", "http://www.clementine-player.org",
  "clementine",
]

print "Running '%s'" % ' '.join(sys.argv)
epydoc.cli.cli()
