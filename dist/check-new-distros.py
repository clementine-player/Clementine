#!/usr/bin/env python3
"""Finds one change CI's Linux distro coverage needs.

Prints it as GitHub Actions outputs (action=add|remove, image=, name=,
branch=), additions first, or prints nothing when CI is up to date.

Supported releases are released, non-EOL Fedora; Ubuntu LTS; and Debian
stable/oldstable. Only releases past end-of-life are removed, never unknown or
unreleased ones.
"""

import datetime
import json
import os
import re
import subprocess
import sys
import urllib.error
import urllib.request

WORKFLOW = os.environ.get('WORKFLOW', '.github/workflows/all.yml')
REMOTE = os.environ.get('REMOTE', 'origin')


def fetch_json(url):
  with urllib.request.urlopen(url, timeout=30) as response:
    return json.load(response)


def release_state(release, today):
  if release['releaseDate'] > today:
    return 'unreleased'
  eol = release['eol']
  if eol is True or (isinstance(eol, str) and eol <= today):
    return 'eol'
  return 'supported'


def known_releases(today):
  """Maps each release's container image to (display name, state)."""
  releases = {}
  for r in fetch_json('https://endoflife.date/api/fedora.json'):
    releases[f"fedora:{r['cycle']}"] = f"Fedora {r['cycle']}", r
  for r in fetch_json('https://endoflife.date/api/ubuntu.json'):
    if r.get('lts'):
      codename = r['codename'].split()[0].lower()
      releases[f'ubuntu:{codename}'] = f"Ubuntu {r['cycle']} {r['codename']}", r
  for r in fetch_json('https://endoflife.date/api/debian.json'):
    codename = r['codename'].lower()
    releases[f'debian:{codename}'] = f"Debian {r['cycle']} {r['codename']}", r
  return {image: (name, release_state(r, today))
          for image, (name, r) in releases.items()}


def image_published(image):
  repo, tag = image.split(':')
  url = f'https://hub.docker.com/v2/namespaces/library/repositories/{repo}/tags/{tag}'
  try:
    with urllib.request.urlopen(url, timeout=30):
      return True
  except urllib.error.HTTPError:
    return False


def branch_exists(branch):
  result = subprocess.run(['git', 'ls-remote', '--heads', REMOTE, branch],
                          capture_output=True, text=True, check=True)
  return bool(result.stdout.strip())


def propose(action, image, name, branch):
  """Prints the change and returns True, unless it's already proposed."""
  if branch_exists(branch):
    return False
  print(f'action={action}\nimage={image}\nname={name}\nbranch={branch}')
  return True


def main():
  today = datetime.datetime.now(datetime.timezone.utc).date().isoformat()
  with open(WORKFLOW) as f:
    built = set(re.findall(r'image: ((?:fedora|ubuntu|debian):[a-z0-9.]+)',
                           f.read()))
  releases = known_releases(today)

  for image, (name, state) in releases.items():
    if (state == 'supported' and image not in built and
        image_published(image) and
        propose('add', image, name, f"distro-watch/{image.replace(':', '-')}")):
      return

  for image in sorted(built):
    if image not in releases:
      print(f"::warning::CI builds {image}, which isn't a supported release",
            file=sys.stderr)
      continue
    name, state = releases[image]
    if state == 'eol' and propose(
        'remove', image, name, f"distro-watch/remove-{image.replace(':', '-')}"):
      return


if __name__ == '__main__':
  main()
