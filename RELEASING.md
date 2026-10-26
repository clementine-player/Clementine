# Releasing

Clementine has development builds, from every change, and numbered releases,
once a week when there's something to tell users. Both are made by GitHub
Actions, with no manual steps.

## Development builds

Every push to `master` builds every platform (`.github/workflows/all.yml`) and
publishes the build:

- as a GitHub **pre-release** named after `git describe`, such as
  `1.4.2-55-g4c062729d`: the source, the Debian, Ubuntu and Fedora packages,
  the macOS DMG, signed and notarized, and the Windows installer;
- in the **apt repository**, for Debian and Ubuntu (`tools/apt-repo`);
- in **COPR**, for Fedora.

People on apt and COPR get every development build. Nobody else is offered
them: the website and the macOS update feed take the numbered releases.

## Numbered releases

**Release notes live in commit messages.** A commit that changes something
users notice ends with a `Release-note:` trailer: one line, written for users
(see "Release notes" in `README.md`). Commits without one (refactoring, tests,
CI, docs) never make a release on their own.

**Every Monday** `.github/workflows/release.yml` collects the notes since the
last release (`dist/release_notes.py`: `dist/release_notes.py plan` shows what
the next release would be). With none, it does nothing. With some, it:

- tags `master` with the next version, as an annotated tag;
- builds every platform from that tag, as `all.yml` builds the development
  builds;
- publishes them as the **latest release** on GitHub, with every note. The
  website offers it, and Clementine Remote's demo and screenshots use it.

Then `.github/workflows/mac-update.yml` puts the DMG in the **macOS update
feed**, with the same notes (`tools/mac-update`). The apt repository and COPR
already have that commit, as a development build.

The notes also go in the **AppStream metadata** that Linux software centres
and Flathub show: CMake fills the `<releases>` of
`dist/org.clementine_player.Clementine.appdata.xml.in` from git with
`dist/release_notes.py appstream`, so nothing is committed for a release. The
source tarball has no git, so `maketarball.sh` packs the metadata as the build
that made the tarball generated it, and builds from the tarball use that.

**Versions.** A release is the last one with its last number one higher:
1.4.2, then 1.4.3. For a new minor or major version, run the workflow by hand
with that version. The version comes from the tag, through `git describe`
(`cmake/Version.cmake`): annotated tags are the numbered releases, and the
development builds' tags are lightweight, so the builds after 1.4.3 are
1.4.3-1-g..., 1.4.3-2-g... and so on.

To release before Monday, run the workflow by hand (*Actions → release → Run
workflow*, on `master`). Run by hand, it releases any change since the last
release: when no commit has a note, the release notes just say "Fixes and
improvements."

**If a release fails** after its tag was pushed (a flaky build, say), run the
workflow again from that tag: *Run workflow*, *Use workflow from*, *Tags*, and
the release. It builds and publishes it again.

### One-time setup

- If tags get a repository ruleset, let GitHub Actions bypass it for numbered
  tags: the workflow pushes them.
