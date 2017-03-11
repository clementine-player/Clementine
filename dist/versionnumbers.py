#!/usr/bin/python2
import rpm
import subprocess

deb_versions = [
  "1.0",
  "1.0.1",
  "1.0.2",
  "1.0.a",
  "1.0~rc1",
  "1.0~rc2",
  "1.0-506-g044287b",
  "1.0-1-g044287b",
]

rpm_versions = [
  ("1.0", "1"),
  ("1.0.1", "1"),
  ("1.0.2", "1"),
  ("1.0", "0.rc1"),
  ("1.0", "0.rc2"),
  ("1.0", "2.506-g044287b"),
]

def compare_deb(left, right):
  if subprocess.call(["dpkg", "--compare-versions", left, "lt", right]) == 0:
    return -1
  if subprocess.call(["dpkg", "--compare-versions", left, "eq", right]) == 0:
    return 0
  if subprocess.call(["dpkg", "--compare-versions", left, "gt", right]) == 0:
    return 1

  raise Exception("Couldn't compare versions %s %s" % (left, right))


def compare_rpm(left, right):
  return rpm.labelCompare(("1", left[0], left[1]), ("1", right[0], right[1]))

def sort_and_print(l, cmp):
  for v in sorted(l, cmp=cmp):
    if isinstance(v, tuple):
      print "%s-%s" % v
    else:
      print v
  print


sort_and_print(deb_versions, compare_deb)
sort_and_print(rpm_versions, compare_rpm)
