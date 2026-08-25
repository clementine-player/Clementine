# macOS code signing

Release builds are signed with a real Apple "Developer ID Application"
certificate and (separately) notarized. Everyone else's builds - local dev,
PRs, forks - fall back to ad-hoc signing automatically (see
`src/CMakeLists.txt`), so none of this is required to build Clementine.

The certificate itself is generated and stored by [fastlane
match](https://docs.fastlane.tools/actions/match/), encrypted, in the
`clementine-macos-signing` GCS bucket. Local maintainers and CI both fetch
and install it the same way, by running `dist/setup_signing.sh` - that's the
one script that does it, so there's no separate "CI process" to keep in
sync with the local one.

Access to the bucket, and to the two secrets it depends on, goes through a
single GCP service account (`mac-signing@clementine-data.iam.gserviceaccount.com`).
Nobody and nothing holds a static key for it: CI impersonates it via
Workload Identity Federation (GitHub OIDC token traded for short-lived GCP
credentials), and maintainers impersonate it via their own Google login.

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

2. **GCP infrastructure** - review and run `dist/gcp_iam_setup.sh` yourself
   (fill in `MAINTAINER_EMAILS` first for anyone who needs local signing).
   It creates the bucket, the two Secret Manager secrets (empty), the
   service account, and the Workload Identity Federation pool/provider
   scoped to only the `clementine-player/Clementine` repo. It prints a
   `workload_identity_provider` value at the end - paste that into
   `.github/workflows/all.yml`'s `build_mac` job (`<GCP_PROJECT_NUMBER>`
   placeholder).

3. **Fill in the two secrets** it created, using the values from step 1:

   ```sh
   # A single JSON blob with all three App Store Connect API key fields:
   cat <<EOF | gcloud secrets versions add apple-appstoreconnect-api-key --project=clementine-data --data-file=-
   {"key_id": "ISSUER_KEY_ID", "issuer_id": "ISSUER_ID", "key": "$(cat AuthKey.p8)"}
   EOF

   # Any strong random passphrase - this is match's own encryption key for
   # the bucket contents, unrelated to Apple:
   openssl rand -base64 32 | gcloud secrets versions add fastlane-match-password --project=clementine-data --data-file=-
   ```

4. Set the `FASTLANE_TEAM_ID` repository variable (Settings → Secrets and
   variables → Actions → Variables) to the Team ID from step 1.

5. Generate the actual certificate once, from a Mac (a maintainer who's just
   been granted impersonation rights, or temporarily run locally by the
   admin who ran step 2):

   ```sh
   gcloud auth application-default login \
     --impersonate-service-account=mac-signing@clementine-data.iam.gserviceaccount.com
   bundle install
   export FASTLANE_TEAM_ID=...   # from step 1 above
   dist/setup_signing.sh renew_signing_cert
   ```

   After this, the everyday `dist/setup_signing.sh` (readonly - it only
   ever fetches) works for everyone with impersonation rights, and for CI.

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

Runs automatically on every push to `master` (not on PRs/forks) - see the
`build_mac` job. Nothing to do.

## Renewing/rotating the certificate

Run the same command as one-time-setup step 5
(`dist/setup_signing.sh renew_signing_cert`). It generates a fresh
certificate and replaces what's stored in the bucket.
