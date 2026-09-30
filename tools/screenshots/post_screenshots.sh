#!/usr/bin/env bash
# Uploads a run's screenshots to the R2 bucket and, for a pull request, posts
# them on it next to master's. Run by .github/workflows/all.yml's Screenshots
# job; one comment per pull request, updated in place. The Android and iOS
# remotes' scripts/post_screenshots.sh do the same for their apps.
#
# Usage: post_screenshots.sh <dir> <pull request number | master>
#
# <dir> has a folder of screenshots per platform (linux, windows, macos). With
# master, they become master's, which later pull requests are compared with:
# the bucket's desktop/master/<platform> names the folder each platform's are
# in. Everything is under desktop/, so the bucket can be the remotes'.
#
# Needs: R2_ACCOUNT_ID, R2_BUCKET, R2_PUBLIC_URL, AWS_ACCESS_KEY_ID and
# AWS_SECRET_ACCESS_KEY (the bucket-scoped R2 token), GITHUB_REPOSITORY,
# GITHUB_RUN_ID, GITHUB_RUN_ATTEMPT, GITHUB_SERVER_URL, GH_TOKEN, and the AWS
# and GitHub CLIs.
set -euo pipefail
# Names in the order the screenshots were taken.
export LC_ALL=C

dir=$1
target=$2
marker='<!-- screenshots -->'
# How wide each screenshot is shown; they're 1280 wide, or twice that on a
# Retina Mac.
width=360

r2() {
  AWS_DEFAULT_REGION=auto \
  AWS_REQUEST_CHECKSUM_CALCULATION=when_required \
  AWS_RESPONSE_CHECKSUM_VALIDATION=when_required \
    aws s3 "$@" --endpoint-url "https://$R2_ACCOUNT_ID.r2.cloudflarestorage.com" --only-show-errors
}
public=${R2_PUBLIC_URL%/}

platforms=()
for platform in linux windows macos; do
  if compgen -G "$dir/$platform/*.png" > /dev/null; then platforms+=("$platform"); fi
done
if [ ${#platforms[@]} -eq 0 ]; then
  echo "No screenshots to post"
  exit 0
fi

# A new folder per run, so GitHub's image cache never shows an earlier run's
# images. They're kept, so old comments keep their images.
if [ "$target" = master ]; then
  prefix="desktop/master/$GITHUB_RUN_ID-$GITHUB_RUN_ATTEMPT"
else
  prefix="desktop/pr-$target/$GITHUB_RUN_ID-$GITHUB_RUN_ATTEMPT"
fi
for platform in "${platforms[@]}"; do
  r2 cp "$dir/$platform" "s3://$R2_BUCKET/$prefix/$platform/" --recursive \
    --exclude '*' --include '*.png' \
    --content-type image/png --cache-control 'public, max-age=2592000, immutable'
done

if [ "$target" = master ]; then
  # Each platform's own pointer: one whose job failed keeps its last ones.
  for platform in "${platforms[@]}"; do
    printf '%s\n' "$prefix/$platform" > "$RUNNER_TEMP/latest"
    r2 cp "$RUNNER_TEMP/latest" "s3://$R2_BUCKET/desktop/master/$platform" \
      --content-type text/plain --cache-control 'no-cache'
    echo "master's $platform screenshots are now $public/$prefix/$platform"
  done
  exit 0
fi

run="$GITHUB_SERVER_URL/$GITHUB_REPOSITORY/actions/runs/$GITHUB_RUN_ID"

# The rows of a table of the screenshots in $1 whose names start with $2.
rows() {
  local platform=$1 kind=$2 shot name before dark
  for shot in "$dir/$platform/$kind"-*.png; do
    [ -e "$shot" ] || continue
    name=$(basename "$shot" .png)
    before="–"
    if [ -n "$master" ] && curl -fsSI "$master/$name.png" > /dev/null 2>&1; then
      before="<img src=\"$master/$name.png\" width=\"$width\">"
    fi
    dark="–"
    if [ -f "$dir/$platform/dark_$name.png" ]; then
      dark="<img src=\"$base/dark_$name.png\" width=\"$width\">"
    fi
    echo "| \`$name\` | $before | <img src=\"$base/$name.png\" width=\"$width\"> | $dark |"
  done
}

header='| Screen | master | This PR | This PR, dark |
| --- | --- | --- | --- |'

{
  echo "$marker"
  echo "### Screenshots"
  echo
  echo "From [run $GITHUB_RUN_ID]($run), with the showcase library. Left: master. Right: this pull request, light and dark."
  open=" open"
  for platform in "${platforms[@]}"; do
    case $platform in
      linux) title="Linux (Fusion)" ;;
      windows) title="Windows" ;;
      macos) title="macOS" ;;
    esac
    base="$public/$prefix/$platform"
    # master's screenshots for this platform, if there are any yet.
    master=
    if latest=$(curl -fsS "$public/desktop/master/$platform?run=$GITHUB_RUN_ID" 2> /dev/null) &&
      [ -n "$latest" ]; then
      master="$public/$latest"
    fi
    echo
    echo "<details$open><summary><b>$title</b></summary>"
    echo
    echo "$header"
    rows "$platform" main
    echo
    echo "<details><summary>Settings</summary>"
    echo
    echo "$header"
    rows "$platform" settings
    echo
    echo "</details>"
    echo "</details>"
    open=
  done
} > "$RUNNER_TEMP/screenshots-comment.md"

existing=$(gh api "repos/$GITHUB_REPOSITORY/issues/$target/comments" --paginate \
  --jq ".[] | select(.user.login == \"github-actions[bot]\" and (.body | startswith(\"$marker\"))) | .id" \
  | head -n 1)
if [ -n "$existing" ]; then
  gh api -X PATCH "repos/$GITHUB_REPOSITORY/issues/comments/$existing" \
    -F "body=@$RUNNER_TEMP/screenshots-comment.md" > /dev/null
else
  gh api "repos/$GITHUB_REPOSITORY/issues/$target/comments" \
    -F "body=@$RUNNER_TEMP/screenshots-comment.md" > /dev/null
fi
