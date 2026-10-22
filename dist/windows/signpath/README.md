# Windows code signing

Release builds for Windows are signed by [SignPath](https://about.signpath.io),
with a certificate from the [SignPath Foundation](https://signpath.org), which
signs open source projects for free. The policy it asks for is on the website:
https://www.clementine-player.org/code-signing.

## What's signed, and how

The `build_mingw` job in `.github/workflows/all.yml` signs Clementine's own
programs and nothing it bundles: the Qt, GStreamer and other DLLs come from
MSYS2, and SignPath's terms don't let us sign other projects' binaries.

1. `clementine.exe`, `clementine-tagreader.exe` and `Uninstall.exe` are
   uploaded as an artifact of the run and signed with the `binaries` artifact
   configuration. The uninstaller has to be a file for this, so the job
   builds it first with `makensis /DUNINSTALLER_ONLY` (see the note at
   `OutFile` in `clementine.nsi.in`).
2. The installer is built around the signed files, with
   `makensis /DPREBUILT_UNINSTALLER`.
3. The installer is uploaded and signed with the `installer` configuration.

SignPath fetches each artifact from the run itself and checks it was built
there, from this repository, on GitHub's own runners.

Pushes to `master` use the `release-signing` policy: Windows trusts its
certificate, and each request waits until an approver approves it on SignPath,
so each release needs two approvals, one for each request. The job waits up to
two hours for each. Pull requests from branches in this repository use
`test-signing`, which signs straight away with a certificate Windows doesn't
trust, so the signing steps are tested before they reach `master`. Fork pull
requests sign nothing.

Every signed file must carry the product name `Clementine` and the build's
version, which the configurations check. `dist/windows/windres.rc.in`,
`clementine-tagreader.rc.in` and the `VIAddVersionKey` lines in
`clementine.nsi.in` set them.

## One-time setup (project admin)

On [SignPath](https://app.signpath.io), in the organization the SignPath
Foundation set up:

1. Turn on multi-factor authentication for everyone in the organization. The
   Foundation requires it, for GitHub too.
2. Trusted build systems: add the predefined **GitHub.com** one and link it to
   the Clementine project.
3. Install the [SignPath GitHub App](https://github.com/apps/signpath) on
   `clementine-player/Clementine`.
4. In the project, add two artifact configurations, pasting in this
   directory's files: slug `binaries` from `binaries.xml`, and slug
   `installer` from `installer.xml`.
5. Check the project has the `test-signing` and `release-signing` signing
   policies, and that `release-signing` needs one approval and lists the
   approvers on the code signing policy page.
6. Make a CI user with the Submitter role on both policies, and an API token
   for it.

On GitHub, in the repository's Settings, Secrets and variables, Actions:

- Secret `SIGNPATH_API_TOKEN`: the token.
- Variable `SIGNPATH_PROJECT_SLUG`: the project's slug.
- Variable `SIGNPATH_ORGANIZATION_ID`: the organization's ID, from its
  settings. Nothing is signed until this is set.

## Approving a release

Every push to `master` makes two signing requests, which SignPath emails the
approvers about. Approve the `binaries` one first: the `installer` one only
appears once it's done. If nobody approves within two hours, the job fails and
that commit has no release; re-running the job makes new requests.
