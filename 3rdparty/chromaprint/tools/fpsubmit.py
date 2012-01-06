#!/usr/bin/env python

import time
import sys
import urllib2
import urllib
import gzip
import socket
from cStringIO import StringIO
from optparse import OptionParser


usage = "usage: %prog [options] logfile"
parser = OptionParser(usage=usage)
parser.add_option("-a", "--api-key", dest="api_key", metavar="KEY",
                  help="your Acoustid API key (http://acoustid.org/api-key)")
parser.add_option("-b", "--batch-size", dest="batch_size", type="int",
                  default=50, metavar="SIZE",
                  help="how many fingerprints to submit in one request [default: %default]")
parser.add_option("--app-url", dest="app_url", type="string",
                  default='http://api.acoustid.org/submit',
                  help="how many fingerprints to submit in one request [default: %default]")
parser.add_option("--app-api-key", dest="app_api_key", type="string", default='5hOby2eZ',
                  help="application API key (needed only if you submit to a non-default URL)")
parser.add_option("-s", "--start", dest="start", type="int",
                  default=1, metavar="SIZE",
                  help="start with the Nth entry from the log file [default: %default]")

(options, args) = parser.parse_args()
if not options.api_key:
    parser.error("no API key specified")
if len(args) != 1:
    parser.error("no log file specified")


USER_API_KEY = options.api_key
CLIENT_API_KEY = options.app_api_key
API_URL = options.app_url
BATCH_SIZE = options.batch_size


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


def encode_params(data):
    encoded_body = StringIO()
    encoded_file = gzip.GzipFile(mode='w', fileobj=encoded_body)
    encoded_file.write(urllib.urlencode(data))
    encoded_file.close()
    return encoded_body.getvalue()


def submit_data(i, entries):
    if not entries:
        return True
    params = { 'user': USER_API_KEY, 'client': CLIENT_API_KEY }
    print 'Submitting... (entries from %d to %d)' % (i, i + len(entries) - 1)
    i = 0
    for entry in [e for e in entries if e['LENGTH'] >= 40 and len(e['FINGERPRINT'])>100]:
        if 'MBID' not in entry and 'PUID' not in entry or int(entry.get('LENGTH', '0')) <= 0:
            continue
        if 'MBID' in entry:
            print '  MBID ', entry['MBID'], entry['FINGERPRINT'][:20] + '...'
            for mbid in entry['MBID'].split(','):
                params['mbid.%d' % i] = mbid
        if 'PUID' in entry:
            print '  PUID ', entry['PUID'], entry['FINGERPRINT'][:20] + '...'
            params['puid.%d' % i] = entry['PUID']
        params['fingerprint.%d' % i] = entry['FINGERPRINT']
        params['length.%d' % i] = entry['LENGTH']
        if 'BITRATE' in entry:
            params['bitrate.%d' % i] = entry['BITRATE']
        if 'FORMAT' in entry:
            params['format.%d' % i] = entry['FORMAT']
        i += 1
    data = encode_params(params)
    request = urllib2.Request(API_URL, data, headers={'Content-Encoding': 'gzip'})
    try:
        urllib2.urlopen(request)
    except urllib2.HTTPError, e:
        print e
        for line in e.readlines():
            print line.rstrip()
        return False
    except urllib2.URLError, e:
        print e
        return False
    except socket.error, e:
        print e
        return False
    print 'OK'
    return True


batch = []
i = options.start
j = 0
for entry in read_log_file(open(args[0]) if args[0] != '-' else sys.stdin):
    j += 1
    if j < options.start:
        continue
    batch.append(entry)
    if len(batch) >= BATCH_SIZE:
        submit_data(i, batch)
        i = j
        batch = []
        time.sleep(0.1)
submit_data(i, batch)

