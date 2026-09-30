# clementine-apt

Publishes the `.deb`s CI builds as an apt repository, so people on Debian and
Ubuntu get Clementine's updates from `apt`. The repository is static files in
the R2 bucket under `apt/`. Its index files are signed with a key in Google
Cloud KMS that can't be exported: CI can ask KMS for signatures, but nobody,
CI included, ever has the key itself.

GnuPG can't use a Cloud KMS key, so this builds the OpenPGP packets itself
around the signatures KMS makes (`openpgp.py`).

## How it's published

The `publish_apt` job in `.github/workflows/all.yml`, on each push to master:

1. Downloads the `.deb`s the release jobs built.
2. Downloads the repository's current index files (not its packages) from R2.
3. `clementine-apt publish` adds the new packages, keeps the newest 10 of each
   in each suite (bookworm, noble...), and writes and signs the index files.
4. Uploads the new packages, then the index files, then deletes the pruned
   packages. Clients that fetched the index just before still find what it
   lists: packages are only deleted once no index names them, and the
   Packages files are also kept under their hash (`Acquire-By-Hash`).

The suite comes from the end of the version: `1.4.1-251-ga588bb3f8~noble` goes
in `noble`.

## Setting it up

1. **Cloud KMS.** Review and run `dist/gcp_apt_signing_setup.sh`, as a project
   admin. It makes the key, a service account that can only sign with it, and
   a Workload Identity Federation provider that only master's pushes can use.

2. **Repository variables** (Settings → Secrets and variables → Actions →
   Variables): `APT_KMS_KEY`, `APT_KEY_CREATED` and `APT_WIF_PROVIDER`, as the
   script prints them. The job is skipped until `APT_KMS_KEY` is set. It uses
   the R2 secrets and variables the screenshots already use.

3. **The fingerprint.** The next push to master publishes the repository and
   the public key, `apt/clementine-archive-keyring.gpg`, and the log prints
   the key's fingerprint ("Signed with …"). Set `APT_KEY_FINGERPRINT` to it:
   from then on a publish with any other key fails, rather than leaving
   everyone who installed the old key unable to update.

The key's creation time is part of its fingerprint, so `APT_KEY_CREATED`
never changes. The public key comes out the same on every run.

## Installing from it

```sh
sudo install -d /etc/apt/keyrings
curl -fsSL <R2_PUBLIC_URL>/apt/clementine-archive-keyring.gpg \
  | sudo tee /etc/apt/keyrings/clementine-archive-keyring.gpg > /dev/null
echo "deb [signed-by=/etc/apt/keyrings/clementine-archive-keyring.gpg] <R2_PUBLIC_URL>/apt $(. /etc/os-release; echo "$VERSION_CODENAME") main" \
  | sudo tee /etc/apt/sources.list.d/clementine.list
sudo apt update && sudo apt install clementine
```

Cloudflare's `r2.dev` addresses are rate limited and not meant for this: give
the bucket a custom domain before telling people about it.

## Development

```sh
uv run ruff check && uv run ruff format --check && uv run ty check
uv run pytest
```

The tests sign with a throwaway local key and check the signatures with
`gpgv`, and with `sqv` (what apt uses from Debian 13 and Ubuntu 25.04) when
it's installed. `--local-key key.pem` in place of `--kms-key` does the same
for trying the whole thing out.
