#!/usr/bin/env bash
# One-time GCP setup for signing the apt repository. NOT run by CI or by
# anyone automatically - a human with admin rights on the clementine-data
# project runs this once. See tools/apt-repo/README.md for the full picture.
#
# Creates:
#   - A Cloud KMS signing key (RSA 4096, PKCS#1 v1.5, SHA-256). It can't be
#     exported: CI asks KMS for signatures and never has the key.
#   - A service account whose only permissions are signing with that key and
#     reading its public half.
#   - A Workload Identity Federation provider, in the pool
#     dist/gcp_iam_setup.sh made, that only pushes to master can use. The
#     service account trusts an attribute only this provider sets, so the mac
#     signing provider's tokens, which pull requests can get, can't sign.
#
# The public key needs a signature too (binding its name to it), so it's
# first written out by CI's first publish, which prints its fingerprint.

set -euo pipefail

PROJECT_ID="clementine-data"
LOCATION="global"
KEY_RING="apt"
KEY="archive-signing"
# software or hsm. Neither can be exported; hsm keys live in FIPS 140-2
# level 3 hardware, at a higher monthly price.
PROTECTION_LEVEL="software"
SERVICE_ACCOUNT_ID="apt-signing"
SERVICE_ACCOUNT_EMAIL="${SERVICE_ACCOUNT_ID}@${PROJECT_ID}.iam.gserviceaccount.com"
WIF_POOL_ID="github-actions"
WIF_PROVIDER_ID="clementine-apt-signing"
GITHUB_REPO="clementine-player/Clementine"
# The only identity allowed to sign: pushes to master.
PUBLISHER="${GITHUB_REPO}@refs/heads/master"

echo "==> Enabling required APIs"
gcloud services enable \
  cloudkms.googleapis.com \
  iam.googleapis.com \
  iamcredentials.googleapis.com \
  sts.googleapis.com \
  --project="$PROJECT_ID"

echo "==> Creating the signing key"
gcloud kms keyrings create "$KEY_RING" \
  --project="$PROJECT_ID" \
  --location="$LOCATION"
gcloud kms keys create "$KEY" \
  --project="$PROJECT_ID" \
  --location="$LOCATION" \
  --keyring="$KEY_RING" \
  --purpose=asymmetric-signing \
  --default-algorithm=rsa-sign-pkcs1-4096-sha256 \
  --protection-level="$PROTECTION_LEVEL"
KEY_NAME="projects/${PROJECT_ID}/locations/${LOCATION}/keyRings/${KEY_RING}/cryptoKeys/${KEY}"
KEY_VERSION="${KEY_NAME}/cryptoKeyVersions/1"

echo "==> Creating the apt-signing service account"
gcloud iam service-accounts create "$SERVICE_ACCOUNT_ID" \
  --project="$PROJECT_ID" \
  --display-name="apt repository signing (Cloud KMS)"

echo "==> Letting it sign with the key and read its public half, and nothing else"
for role in roles/cloudkms.signer roles/cloudkms.publicKeyViewer; do
  gcloud kms keys add-iam-policy-binding "$KEY" \
    --project="$PROJECT_ID" \
    --location="$LOCATION" \
    --keyring="$KEY_RING" \
    --member="serviceAccount:${SERVICE_ACCOUNT_EMAIL}" \
    --role="$role"
done

echo "==> Creating a Workload Identity Federation provider for master's pushes"
gcloud iam workload-identity-pools providers create-oidc "$WIF_PROVIDER_ID" \
  --project="$PROJECT_ID" \
  --location=global \
  --workload-identity-pool="$WIF_POOL_ID" \
  --display-name="Clementine apt signing" \
  --issuer-uri="https://token.actions.githubusercontent.com" \
  --attribute-mapping="google.subject=assertion.sub,attribute.repository=assertion.repository,attribute.apt_publisher=assertion.repository + '@' + assertion.ref" \
  --attribute-condition="assertion.repository == '${GITHUB_REPO}' && assertion.ref == 'refs/heads/master' && assertion.event_name == 'push'"

PROJECT_NUMBER="$(gcloud projects describe "$PROJECT_ID" --format='value(projectNumber)')"

echo "==> Allowing only that provider's master pushes to impersonate the service account"
gcloud iam service-accounts add-iam-policy-binding "$SERVICE_ACCOUNT_EMAIL" \
  --project="$PROJECT_ID" \
  --role="roles/iam.workloadIdentityUser" \
  --member="principalSet://iam.googleapis.com/projects/${PROJECT_NUMBER}/locations/global/workloadIdentityPools/${WIF_POOL_ID}/attribute.apt_publisher/${PUBLISHER}"

# The key's creation time is part of its OpenPGP fingerprint, so it's fixed
# here, once, as whole seconds.
CREATED_AT="$(gcloud kms keys versions describe 1 \
  --project="$PROJECT_ID" \
  --location="$LOCATION" \
  --keyring="$KEY_RING" \
  --key="$KEY" \
  --format='value(createTime)')"
CREATED="$(python3 -c 'import datetime, sys
print(int(datetime.datetime.fromisoformat(sys.argv[1][:19] + "+00:00").timestamp()))' "$CREATED_AT")"

cat <<EOF
==> Done. Set these repository variables (Settings → Secrets and variables →
    Actions → Variables):
    APT_KMS_KEY=${KEY_VERSION}
    APT_KEY_CREATED=${CREATED}
    APT_WIF_PROVIDER=projects/${PROJECT_NUMBER}/locations/global/workloadIdentityPools/${WIF_POOL_ID}/providers/${WIF_PROVIDER_ID}

    The next push to master publishes the repository and the public key
    (apt/clementine-archive-keyring.gpg in the bucket), and its log prints the
    key's fingerprint. Set APT_KEY_FINGERPRINT to it, so a later publish with
    any other key fails instead of locking everyone out.
EOF
