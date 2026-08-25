#!/usr/bin/env bash
# Installs the Clementine Developer ID signing certificate into the current
# keychain and prints the resulting codesign identity on stdout. This is the
# ONE script both local maintainers (with real certs) and CI use - see
# dist/CODE_SIGNING.md. All progress output goes to stderr so the identity
# on stdout can be captured directly:
#
#   APPLE_DEVELOPER_ID="$(dist/setup_signing.sh)"
#
# Pass "renew_signing_cert" as the first argument to generate a new
# certificate instead of fetching the existing one (maintainers only, see
# dist/CODE_SIGNING.md) - everything else about the process is identical
# either way, since fastlane match always talks to the App Store Connect API
# to confirm the certificate it's about to install is still valid, whether
# it's fetching or generating one.
#
# Requires: gcloud authenticated (locally: impersonating the mac-signing
# service account; in CI: via Workload Identity Federation) and `bundle
# install` already run. FASTLANE_TEAM_ID must be set.
set -euo pipefail

LANE="${1:-signing_cert}"
PROJECT_ID="clementine-data"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

API_KEY_PATH="$(mktemp)"
trap 'rm -f "$API_KEY_PATH"' EXIT

echo "Fetching App Store Connect API key from Secret Manager..." >&2
gcloud secrets versions access latest \
  --project="$PROJECT_ID" \
  --secret=apple-appstoreconnect-api-key \
  > "$API_KEY_PATH"

export APP_STORE_CONNECT_API_KEY_PATH="$API_KEY_PATH"

echo "Installing signing certificate via fastlane ($LANE)..." >&2
bundle exec fastlane "$LANE" 1>&2

APPLE_DEVELOPER_ID="$(security find-identity -v -p codesigning \
  | grep -o '"Developer ID Application[^"]*"' | head -1 | tr -d '"')"

if [[ -z "$APPLE_DEVELOPER_ID" ]]; then
  echo "No 'Developer ID Application' identity found in the keychain after fastlane match - see dist/CODE_SIGNING.md." >&2
  exit 1
fi

if [[ -n "${GITHUB_ENV:-}" ]]; then
  echo "APPLE_DEVELOPER_ID=$APPLE_DEVELOPER_ID" >> "$GITHUB_ENV"
fi

echo "$APPLE_DEVELOPER_ID"
