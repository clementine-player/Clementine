#!/usr/bin/env python

import time
import sys
from optparse import OptionParser
import subprocess
import os.path
from xml.etree import ElementTree


usage = "usage: %prog [options] logfile"
parser = OptionParser(usage=usage)
parser.add_option("-a", "--musicdns-apikey", dest="musicdns_api_key", metavar="KEY",
                  help="MusicDNS API key")
parser.add_option("-g", "--genpuid", dest="genpuid_path", metavar="PATH",
                  help="path to the GenPUID binary", default="genpuid")

(options, args) = parser.parse_args()
if len(args) != 1:
    parser.error("no log file specified")


def read_log_file(input):
    group = {}
    for line in input:
        line = line.strip()
        if not line:
            if group:
                yield group
            group = {}
            continue
        name, value = line.split('=', 1)
        group[name] = value
    if group:
        yield group


def make_groups(input, size=10):
    group = []
    for entry in input:
        group.append(entry)
        if len(group) >= size:
            yield group
            group = []
    if group:
        yield group



def write_log_file(entry):
    for row in entry.iteritems():
        print '%s=%s' % row
    print


def call_genpuid(entries):
    paths = [e['FILENAME'] for e in entries]
    process = subprocess.Popen([options.genpuid_path, options.musicdns_api_key, '-xml'] + paths, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    tree = ElementTree.fromstring('<?xml version="1.0" encoding="iso-8859-1"?>' + out)
    puids = {}
    for track in tree.findall("track"):
        if 'puid' in track.attrib and 'file' in track.attrib:
            puids[os.path.normpath(track.attrib['file']).encode('iso-8859-1')] = track.attrib['puid']
    for entry in entries:
        path = os.path.normpath(entry['FILENAME'])
        if path in puids:
            entry['PUID'] = puids[path]


for entries in make_groups(read_log_file(open(args[0]) if args[0] != '-' else sys.stdin), 20):
    call_genpuid([e for e in entries if 'MBID' not in e])
    for entry in entries:
        print >>sys.stderr, entry['FILENAME']
        write_log_file(entry)

