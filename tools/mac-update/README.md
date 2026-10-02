# clementine-mac-update

Sparkle updates for the macOS build: `clementine-mac-update` downloads Sparkle
for the build (`fetch-sparkle`), decides what goes in the update feed
(`plan`), and puts it there (`publish`).

## How updates work

The macOS app updates itself with [Sparkle 2](https://sparkle-project.org/).
It checks the feed at `https://data.clementine-player.org/sparkle2`
(`SUFeedURL` in `dist/Info.plist.in`), served by the data.clementine-player.org
app in [clementine-player/Website](https://github.com/clementine-player/Website)
from the `MacUpdate` entities in the `clementine-data` project's Datastore,
one per published build. For each minimum macOS, the feed offers the most
recently published one, which is the highest build for it: Sparkle offers the
highest build a Mac can run, so `publish` refuses a lower build than the
newest for its macOS.

Builds from before Sparkle 2 check the old feed, `/sparkle`, and never see
these updates: they're Intel-only, and the DMG is arm64-only.

## What's in the feed

Every push to `master` makes a GitHub release with a notarized DMG
(`all.yml`). The feed gets one of them a week at most, and only when there's
something to tell users.

**Release notes live in commit messages.** A commit that changes something
users notice ends with a `Release-note:` trailer: one line, written for users.
Commits without one never make an update on their own.

**Every Monday** `.github/workflows/mac-update.yml` collects the notes of the
commits since the last update (`clementine-mac-update plan`; run it to see
what the next update would be). With none, it does nothing. With some, it takes the
newest release on `master` with a DMG, and:

- signs the DMG for Sparkle (EdDSA) with `sign_update`;
- checks the signature against the `SUPublicEDKey` in the app inside it, the
  key installs check updates with;
- adds it to the feed with the notes (`clementine-mac-update publish`), and the
  version and minimum macOS from the app's own `Info.plist`: Sparkle compares
  `CFBundleVersion`, and doesn't offer an update to an older macOS than
  `LSMinimumSystemVersion`;
- tags the release's commit `sparkle/<release>`, which is where the next
  update's notes start.

To publish before Monday, run the workflow by hand (*Actions → mac-update →
Run workflow*, on `master`). Run by hand, it publishes any change since the
last update: when no commit has a note, the notes just say "Fixes and
improvements."

`LSMinimumSystemVersion` is the macOS the DMG was built on (see
`dist/CMakeLists.txt`): the app bundles Homebrew's libraries, which are built
for the macOS they're installed on. When CI's runner moves to a newer macOS,
so does the minimum, and Sparkle stops offering updates to older Macs.

## The signing key

The EdDSA private key is in Secret Manager (`sparkle-ed-private-key` in
`clementine-data`). Only the `sparkle-feed` service account can read it, and
only `mac-update.yml` running on `master` can act as that account, through
Workload Identity Federation: no key or credential is stored in GitHub.

The public key is in `dist/sparkle_public_key.txt`, which CMake puts in the
app's `Info.plist` (`SUPublicEDKey`). Sparkle is left out of builds until that
file exists.

Every install checks updates with the key it was built with: Sparkle accepts
an update signed with that key, or one code signed by the same Developer ID
team as the installed app. So one of the two can change in an update while
the other stays the same, never both at once.

**Rotating the key** (or replacing a lost one): make a new key pair, add its
private key as a new version of the secret, and commit its public key in
`dist/sparkle_public_key.txt`, at the same time. The next update is signed
with the new key, installs accept it for its Developer ID signature, and
check later updates with the new key. `clementine-mac-update publish` checks
each update against the key in its own app, so a build with the old key isn't
signed with the new one by mistake.

The key alone is enough to make an update installs accept (Sparkle requires
an update to be code signed, but ad-hoc signing passes), so keep it where it
is: only the feed workflow can read it.

## One-time setup

1. Review `dist/gcp_sparkle_setup.sh` and run it as an admin of
   `clementine-data` (it needs `brew install openssl@3`). It makes the key
   pair, puts the private key in Secret Manager, and creates the service
   account and its Workload Identity Federation provider.
2. Commit the public key it writes to `dist/sparkle_public_key.txt`.
3. Set the repository variable `SPARKLE_WIF_PROVIDER` that it prints. Until
   it's set, `mac-update.yml` is skipped.
4. If `sparkle/*` tags get a repository ruleset, let GitHub Actions bypass it:
   the workflow pushes them.
5. Once a DMG built with the key is released, run the workflow by hand for
   the first update.

## Building locally

```sh
uv run --project tools/mac-update clementine-mac-update fetch-sparkle <directory>
```

downloads the pinned Sparkle release, as CI does (to move to a newer one, see
`sparkle.py`). Pass that directory to CMake with `-DCMAKE_FRAMEWORK_PATH`.
Without it the app builds without Sparkle and its *Check for updates* menu
item.

## Development

From this directory:

```sh
uv run ruff check && uv run ruff format --check && uv run ty check
uv run pytest
```

To see what the next update would be, from the repository's root (it needs the
GitHub CLI, to look at the releases):

```sh
uv run --project tools/mac-update clementine-mac-update plan --notes /dev/stdout
```
