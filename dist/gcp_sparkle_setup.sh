#!/usr/bin/env bash
# One-time GCP setup for publishing macOS updates to the Sparkle feed. NOT
# run by CI or by anyone automatically - a human with admin rights on the
# clementine-data project runs this once. See tools/mac-update/README.md for the
# full picture.
#
# Creates:
#   - Sparkle's EdDSA (ed25519) key pair. The private key goes straight into
#     a Secret Manager secret; it's never written to disk here, and nobody
#     needs a copy. The public key goes in dist/sparkle_public_key.txt, to
#     commit.
#   - A service account whose only permissions are reading that secret and
#     writing the feed's entries to Datastore.
#   - A Workload Identity Federation provider, in the pool
#     dist/gcp_iam_setup.sh made, that only .github/workflows/mac-update.yml
#     on master can use. The service account trusts an attribute only this
#     provider sets, so the other providers' tokens (the mac signing one,
#     which pull requests can get) can't reach the key.
#
# Needs an OpenSSL with ed25519, which macOS's own LibreSSL lacks:
# brew install openssl@3, or set OPENSSL.

set -euo pipefail

PROJECT_ID="clementine-data"
SECRET="sparkle-ed-private-key"
SERVICE_ACCOUNT_ID="sparkle-feed"
SERVICE_ACCOUNT_EMAIL="${SERVICE_ACCOUNT_ID}@${PROJECT_ID}.iam.gserviceaccount.com"
WIF_POOL_ID="github-actions"
WIF_PROVIDER_ID="clementine-sparkle-feed"
GITHUB_REPO="clementine-player/Clementine"
# The only identity allowed to publish: the feed workflow, on master.
PUBLISHER="${GITHUB_REPO}/.github/workflows/mac-update.yml@refs/heads/master"
PUBLIC_KEY_FILE="$(dirname "$0")/sparkle_public_key.txt"

OPENSSL="${OPENSSL:-$(brew --prefix openssl@3 2>/dev/null)/bin/openssl}"
[[ -x "$OPENSSL" ]] || OPENSSL=openssl

echo "==> Generating the signing key"
# A PKCS#8 ed25519 key is 48 bytes, its last 32 the seed, which is what
# Sparkle's sign_update takes (base64). The SubjectPublicKeyInfo is 44 bytes,
# its last 32 the public key, which is what SUPublicEDKey takes (base64).
key_der="$("$OPENSSL" genpkey -algorithm ed25519 -outform DER | base64)"
if [[ "$(base64 -d <<<"$key_der" | wc -c | tr -d ' ')" != 48 ]]; then
  echo "$OPENSSL can't make ed25519 keys; set OPENSSL to one that can." >&2
  exit 1
fi
private_key="$(base64 -d <<<"$key_der" | tail -c 32 | base64)"
public_key="$(base64 -d <<<"$key_der" | "$OPENSSL" pkey -inform DER -pubout -outform DER | tail -c 32 | base64)"
unset key_der

if [[ -e "$PUBLIC_KEY_FILE" ]]; then
  echo "$PUBLIC_KEY_FILE already exists: to change the key, see tools/mac-update/README.md." >&2
  exit 1
fi

echo "==> Enabling required APIs"
gcloud services enable \
  datastore.googleapis.com \
  iam.googleapis.com \
  iamcredentials.googleapis.com \
  secretmanager.googleapis.com \
  sts.googleapis.com \
  --project="$PROJECT_ID"

echo "==> Storing the private key in Secret Manager"
gcloud secrets create "$SECRET" --project="$PROJECT_ID" --replication-policy=automatic
printf '%s' "$private_key" |
  gcloud secrets versions add "$SECRET" --project="$PROJECT_ID" --data-file=-
unset private_key
echo "$public_key" > "$PUBLIC_KEY_FILE"

echo "==> Creating the sparkle-feed service account"
gcloud iam service-accounts create "$SERVICE_ACCOUNT_ID" \
  --project="$PROJECT_ID" \
  --display-name="Sparkle update feed (signing key + Datastore)"

echo "==> Letting it read the key and write the feed"
gcloud secrets add-iam-policy-binding "$SECRET" \
  --project="$PROJECT_ID" \
  --member="serviceAccount:${SERVICE_ACCOUNT_EMAIL}" \
  --role="roles/secretmanager.secretAccessor"
# Datastore roles can't be narrowed to a kind, so this is the project's
# whole Datastore: the data.clementine-player.org app's too.
gcloud projects add-iam-policy-binding "$PROJECT_ID" \
  --member="serviceAccount:${SERVICE_ACCOUNT_EMAIL}" \
  --role="roles/datastore.user" \
  --condition=None

echo "==> Creating a Workload Identity Federation provider for the feed workflow"
gcloud iam workload-identity-pools providers create-oidc "$WIF_PROVIDER_ID" \
  --project="$PROJECT_ID" \
  --location=global \
  --workload-identity-pool="$WIF_POOL_ID" \
  --display-name="Clementine Sparkle feed" \
  --issuer-uri="https://token.actions.githubusercontent.com" \
  --attribute-mapping="google.subject=assertion.sub,attribute.sparkle_publisher=assertion.workflow_ref" \
  --attribute-condition="assertion.repository == '${GITHUB_REPO}' && assertion.ref == 'refs/heads/master' && assertion.workflow_ref == '${PUBLISHER}'"

PROJECT_NUMBER="$(gcloud projects describe "$PROJECT_ID" --format='value(projectNumber)')"

echo "==> Allowing only that workflow to impersonate the service account"
gcloud iam service-accounts add-iam-policy-binding "$SERVICE_ACCOUNT_EMAIL" \
  --project="$PROJECT_ID" \
  --role="roles/iam.workloadIdentityUser" \
  --member="principalSet://iam.googleapis.com/projects/${PROJECT_NUMBER}/locations/global/workloadIdentityPools/${WIF_POOL_ID}/attribute.sparkle_publisher/${PUBLISHER}"

cat <<EOF
==> Done.
    1. Commit the public key, which is now in ${PUBLIC_KEY_FILE}.
    2. Set this repository variable (Settings → Secrets and variables →
       Actions → Variables):
         SPARKLE_WIF_PROVIDER=projects/${PROJECT_NUMBER}/locations/global/workloadIdentityPools/${WIF_POOL_ID}/providers/${WIF_PROVIDER_ID}

    Don't delete the secret or its versions: to change the key, see
    "Rotating the key" in tools/mac-update/README.md. This script makes it once.
EOF
