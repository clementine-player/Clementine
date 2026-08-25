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
ADC_PATCHED_PATH="$(mktemp)"
trap 'rm -f "$API_KEY_PATH" "$ADC_PATCHED_PATH"' EXIT

# renew_signing_cert deliberately needs interactive Apple ID auth instead
# (see fastlane/Fastfile) - match's api_key_path option falls back to
# reading this env var even when a lane doesn't pass it explicitly in code,
# so it has to be genuinely unset here (not just left unexported by us -
# it may already be exported in the calling shell from earlier testing).
if [[ "$LANE" == "renew_signing_cert" ]]; then
  unset APP_STORE_CONNECT_API_KEY_PATH
else
  echo "Fetching App Store Connect API key from Secret Manager..." >&2
  gcloud secrets versions access latest \
    --project="$PROJECT_ID" \
    --secret=apple-appstoreconnect-api-key \
    > "$API_KEY_PATH"

  export APP_STORE_CONNECT_API_KEY_PATH="$API_KEY_PATH"
fi

# match's own Application Default Credentials auto-detection calls
# Google::Auth.get_application_default with no scope, which crashes
# specifically for locally-impersonated ADC (gcloud auth
# application-default login --impersonate-service-account=...) since that
# credential type fails fast without one - silently, since match swallows
# the exception and falls back to its interactive "create a gc_keys.json"
# flow. Pointing match at the credentials file directly instead skips that
# broken pre-check; the actual GCS client library loads the same file
# correctly (with a proper scope). Works for both: locally this is gcloud's
# well-known ADC path, in CI it's whatever google-github-actions/auth wrote.
#
# match separately insists that whatever keys file it's given contains a
# top-level "project_id" field (true of a real downloaded service account
# key, not of an ADC/impersonation credentials file) - so patch a copy
# rather than pointing at the ADC file directly.
ADC_SOURCE_PATH="${GOOGLE_APPLICATION_CREDENTIALS:-$HOME/.config/gcloud/application_default_credentials.json}"
PROJECT_ID="$PROJECT_ID" ADC_SOURCE_PATH="$ADC_SOURCE_PATH" ADC_PATCHED_PATH="$ADC_PATCHED_PATH" python3 -c '
import json, os
with open(os.environ["ADC_SOURCE_PATH"]) as f:
    creds = json.load(f)
creds.setdefault("project_id", os.environ["PROJECT_ID"])
with open(os.environ["ADC_PATCHED_PATH"], "w") as f:
    json.dump(creds, f)
'
export MATCH_GOOGLE_CLOUD_KEYS_FILE="$ADC_PATCHED_PATH"

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
