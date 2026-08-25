#!/usr/bin/env bash
# One-time GCP infrastructure setup for macOS code signing. NOT run by CI or
# by anyone automatically - a human with admin rights on the clementine-data
# project runs this once. See dist/CODE_SIGNING.md for the full picture.
#
# Creates:
#   - A GCS bucket holding the fastlane-match-encrypted signing certificate
#   - Two empty Secret Manager secrets (values added separately, by hand -
#     see dist/CODE_SIGNING.md, since this script shouldn't handle raw
#     credential material)
#   - A dedicated service account that's the ONLY identity with direct
#     access to the bucket/secrets
#   - Workload Identity Federation so GitHub Actions (build_mac, pushes to
#     master only) can impersonate that service account without any
#     long-lived key ever existing
#   - Impersonation rights for named maintainers, so local signing uses the
#     exact same service-account identity as CI
#
# Review before running: fill in MAINTAINER_EMAILS below first.
set -euo pipefail

PROJECT_ID="clementine-data"
BUCKET_NAME="clementine-macos-signing"
SERVICE_ACCOUNT_ID="mac-signing"
SERVICE_ACCOUNT_EMAIL="${SERVICE_ACCOUNT_ID}@${PROJECT_ID}.iam.gserviceaccount.com"
WIF_POOL_ID="github-actions"
WIF_PROVIDER_ID="clementine-mac-signing"
GITHUB_REPO="clementine-player/Clementine"

# Fill in before running: Google accounts of maintainers who hold a real
# Developer ID cert and need to sign locally.
MAINTAINER_EMAILS=(
  "john.maguire@gmail.com"
)

echo "==> Enabling required APIs"
gcloud services enable \
  iam.googleapis.com \
  iamcredentials.googleapis.com \
  secretmanager.googleapis.com \
  storage.googleapis.com \
  sts.googleapis.com \
  --project="$PROJECT_ID"

echo "==> Creating signing-certificate bucket"
gcloud storage buckets create "gs://${BUCKET_NAME}" \
  --project="$PROJECT_ID" \
  --location=us \
  --uniform-bucket-level-access
gcloud storage buckets update "gs://${BUCKET_NAME}" --versioning

echo "==> Creating Secret Manager secrets (empty - add values by hand, see dist/CODE_SIGNING.md)"
gcloud secrets create apple-appstoreconnect-api-key --project="$PROJECT_ID" --replication-policy=automatic
gcloud secrets create fastlane-match-password --project="$PROJECT_ID" --replication-policy=automatic

echo "==> Creating mac-signing service account"
gcloud iam service-accounts create "$SERVICE_ACCOUNT_ID" \
  --project="$PROJECT_ID" \
  --display-name="macOS code signing (fastlane match + Secret Manager)"

echo "==> Granting the service account access to its bucket and secrets"
gcloud storage buckets add-iam-policy-binding "gs://${BUCKET_NAME}" \
  --member="serviceAccount:${SERVICE_ACCOUNT_EMAIL}" \
  --role="roles/storage.objectAdmin"
gcloud secrets add-iam-policy-binding apple-appstoreconnect-api-key \
  --project="$PROJECT_ID" \
  --member="serviceAccount:${SERVICE_ACCOUNT_EMAIL}" \
  --role="roles/secretmanager.secretAccessor"
gcloud secrets add-iam-policy-binding fastlane-match-password \
  --project="$PROJECT_ID" \
  --member="serviceAccount:${SERVICE_ACCOUNT_EMAIL}" \
  --role="roles/secretmanager.secretAccessor"

echo "==> Creating Workload Identity Pool + Provider for GitHub Actions"
gcloud iam workload-identity-pools create "$WIF_POOL_ID" \
  --project="$PROJECT_ID" \
  --location=global \
  --display-name="GitHub Actions"

gcloud iam workload-identity-pools providers create-oidc "$WIF_PROVIDER_ID" \
  --project="$PROJECT_ID" \
  --location=global \
  --workload-identity-pool="$WIF_POOL_ID" \
  --display-name="Clementine mac signing" \
  --issuer-uri="https://token.actions.githubusercontent.com" \
  --attribute-mapping="google.subject=assertion.sub,attribute.repository=assertion.repository" \
  --attribute-condition="assertion.repository == '${GITHUB_REPO}'"

PROJECT_NUMBER="$(gcloud projects describe "$PROJECT_ID" --format='value(projectNumber)')"

echo "==> Allowing that provider to impersonate the service account"
gcloud iam service-accounts add-iam-policy-binding "$SERVICE_ACCOUNT_EMAIL" \
  --project="$PROJECT_ID" \
  --role="roles/iam.workloadIdentityUser" \
  --member="principalSet://iam.googleapis.com/projects/${PROJECT_NUMBER}/locations/global/workloadIdentityPools/${WIF_POOL_ID}/attribute.repository/${GITHUB_REPO}"

if [[ ${#MAINTAINER_EMAILS[@]} -gt 0 ]]; then
  echo "==> Granting named maintainers impersonation rights on the service account"
  for email in "${MAINTAINER_EMAILS[@]}"; do
    gcloud iam service-accounts add-iam-policy-binding "$SERVICE_ACCOUNT_EMAIL" \
      --project="$PROJECT_ID" \
      --role="roles/iam.serviceAccountTokenCreator" \
      --member="user:${email}"
  done
else
  echo "==> No MAINTAINER_EMAILS set - skipping local-impersonation grants. Re-run add-iam-policy-binding manually per maintainer later:"
  echo "    gcloud iam service-accounts add-iam-policy-binding ${SERVICE_ACCOUNT_EMAIL} --project=${PROJECT_ID} --role=roles/iam.serviceAccountTokenCreator --member=user:EMAIL"
fi

cat <<EOF

==> Done. For the GitHub Actions workflow, use:
    workload_identity_provider: projects/${PROJECT_NUMBER}/locations/global/workloadIdentityPools/${WIF_POOL_ID}/providers/${WIF_PROVIDER_ID}
    service_account: ${SERVICE_ACCOUNT_EMAIL}
EOF
