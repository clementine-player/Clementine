# macOS code signing

Release builds are signed with a real Apple "Developer ID Application"
certificate and (separately) notarized. Everyone else's builds - local dev,
PRs, forks - fall back to ad-hoc signing automatically (see
`src/CMakeLists.txt`), so none of this is required to build Clementine.

The certificate itself is generated and stored by [fastlane
match](https://docs.fastlane.tools/actions/match/) in the
`clementine-macos-signing` GCS bucket. Local maintainers and CI both fetch
and install it the same way, by running `dist/setup_signing.sh` - that's the
one script that does it, so there's no separate "CI process" to keep in
sync with the local one.

Unlike match's git/S3 backends, its `google_cloud` storage mode doesn't
encrypt the certificate client-side - confidentiality comes entirely from
the bucket's IAM. That's why access to it, and to the one secret it depends
on, goes through a single GCP service account
(`mac-signing@clementine-data.iam.gserviceaccount.com`). Nobody and nothing
holds a static key for it: CI impersonates it via Workload Identity
Federation (GitHub OIDC token traded for short-lived GCP credentials), and
maintainers impersonate it via their own Google login.

## One-time setup (project admin)

1. **Apple App Store Connect API key** - the one step that has to happen by
   hand, since it needs an Apple ID login:
   - developer.apple.com → Users and Access → Integrations → App Store
     Connect API → generate a key with the "Developer" role (enough to
     manage certificates) or "Admin" if unavailable.
   - Note the **Issuer ID**, **Key ID**, and download the `.p8` private key
     (Apple only lets you download it once).
   - Also note your **Team ID** (Membership page).
   - This key doesn't expire on its own - rotate it yourself periodically
     by revoking and re-generating.

   While you're on the portal: match unconditionally verifies that
   `app_identifier` (Clementine's real bundle ID) is a registered App ID,
   regardless of cert type - register it if it isn't already: Certificates,
   IDs & Profiles → Identifiers → **+** → App IDs → App → Explicit bundle ID
   `org.clementine-player.clementine` → Register. No special capabilities
   needed, this is only for Developer ID signing, not App Store
   distribution.

2. **GCP infrastructure** - review and run `dist/gcp_iam_setup.sh` yourself
   (fill in `MAINTAINER_EMAILS` first for anyone who needs local signing).
   It creates the bucket, the empty Secret Manager secret, the service
   account, and the Workload Identity Federation pool/provider scoped to
   only the `clementine-player/Clementine` repo. It prints a
   `workload_identity_provider` value at the end - paste that into
   `.github/workflows/all.yml`'s `build_mac` job (`<GCP_PROJECT_NUMBER>`
   placeholder).

3. **Fill in the secret** it created, using the values from step 1 - a
   single JSON blob with all three App Store Connect API key fields. Build
   it with `json.dumps` rather than a shell heredoc - splicing the `.p8`
   file's raw content into a quoted string leaves its newlines unescaped,
   which produces invalid JSON:

   ```sh
   KEY_ID=ISSUER_KEY_ID ISSUER_ID=ISSUER_ID AUTH_KEY_PATH=AuthKey.p8 python3 -c '
   import json, os
   with open(os.environ["AUTH_KEY_PATH"]) as f:
       key = f.read()
   print(json.dumps({"key_id": os.environ["KEY_ID"], "issuer_id": os.environ["ISSUER_ID"], "key": key}))
   ' | gcloud secrets versions add apple-appstoreconnect-api-key --project=clementine-data --data-file=-
   ```

4. Set the `FASTLANE_TEAM_ID` repository variable (Settings → Secrets and
   variables → Actions → Variables) to the Team ID from step 1.

5. Generate the actual certificate once. Apple restricts *creating* a
   Developer ID certificate to the account's Account Holder - not even
   Admin is enough - and (per fastlane's own source) that operation isn't
   reliably available through API key auth regardless of the key's role,
   so this one step needs an interactive Apple ID login (password + 2FA -
   can't be scripted, run it in a real terminal) as the Account Holder:

   ```sh
   gcloud auth application-default login \
     --impersonate-service-account=mac-signing@clementine-data.iam.gserviceaccount.com
   bundle install
   export FASTLANE_TEAM_ID=...       # from step 1 above
   export FASTLANE_USER=you@example.com   # Account Holder's Apple ID
   dist/setup_signing.sh renew_signing_cert
   ```

   After this, the everyday `dist/setup_signing.sh` (readonly, API-key
   auth, no 2FA prompt - it only ever fetches) works for everyone with
   impersonation rights, and for CI.

## Local signing (maintainers with a real cert)

```sh
gcloud auth application-default login \
  --impersonate-service-account=mac-signing@clementine-data.iam.gserviceaccount.com
bundle install
export FASTLANE_TEAM_ID=...   # from step 1 above

APPLE_DEVELOPER_ID="$(dist/setup_signing.sh)"
cmake .. -DAPPLE_DEVELOPER_ID="$APPLE_DEVELOPER_ID"
```

## CI

Runs automatically on every push to `master` and on PRs opened from
branches within this repo (not forks) - see the `Check signing eligibility`
step in the `build_mac` job. Nothing to do.

This also notarizes the build (submits the signed app and dmg to Apple's
notary service and staples the ticket) automatically - see
`APPLE_NOTARIZE_API_KEY_PATH` in `src/CMakeLists.txt`/`dist/notarize.py` and
the `Notarize DMG` step. `dist/setup_signing.sh` only wires that variable up
in CI (via `$GITHUB_ENV`); local builds stay ad-hoc/Developer-ID-signed but
unnotarized by default. To test notarization locally, pass
`-DAPPLE_NOTARIZE_API_KEY_PATH=<path to the same API key JSON>` to `cmake`
yourself.

## Renewing/rotating the certificate

Run the same command as one-time-setup step 5. Must be the Account Holder,
with `FASTLANE_USER` set to their Apple ID - expect an interactive
password + 2FA prompt. Generates a fresh certificate and replaces what's
stored in the bucket.
