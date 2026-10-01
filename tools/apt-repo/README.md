# clementine-apt

Publishes the `.deb`s CI builds as an apt repository, so people on Debian and
Ubuntu get Clementine's updates from `apt`. The repository is static files at
the root of an R2 bucket of its own. Its index files are signed with a key in
Google Cloud KMS that can't be exported: CI can ask KMS for signatures, but nobody,
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

2. **The bucket.** Create an R2 bucket for the repository, give it a custom
   domain (`r2.dev` addresses are rate limited, and the domain is what people
   put in their sources), and make an R2 API token that can only read and
   write that bucket.

3. **Repository settings** (Settings → Secrets and variables → Actions):
   - variables `APT_KMS_KEY`, `APT_KEY_CREATED` and `APT_WIF_PROVIDER`, as the
     script prints them, and `APT_R2_BUCKET`, the bucket's name;
   - secrets `APT_R2_ACCESS_KEY_ID` and `APT_R2_SECRET_ACCESS_KEY`, the
     bucket's token. `R2_ACCOUNT_ID` is the account's, as for the
     screenshots.

   The job is skipped until `APT_KMS_KEY` and `APT_R2_BUCKET` are set.

4. **The fingerprint.** The next push to master publishes the repository and
   the public key, `clementine-archive-keyring.gpg`, and the log prints
   the key's fingerprint ("Signed with …"). Set `APT_KEY_FINGERPRINT` to it:
   from then on a publish with any other key fails, rather than leaving
   everyone who installed the old key unable to update.

The key's creation time is part of its fingerprint, so `APT_KEY_CREATED`
never changes. The public key comes out the same on every run.

## Installing from it

```sh
sudo install -d /etc/apt/keyrings
curl -fsSL https://<domain>/clementine-archive-keyring.gpg \
  | sudo tee /etc/apt/keyrings/clementine-archive-keyring.gpg > /dev/null
echo "deb [signed-by=/etc/apt/keyrings/clementine-archive-keyring.gpg] https://<domain> $(. /etc/os-release; echo "$VERSION_CODENAME") main" \
  | sudo tee /etc/apt/sources.list.d/clementine.list
sudo apt update && sudo apt install clementine
```

## Development

```sh
uv run ruff check && uv run ruff format --check && uv run ty check
uv run pytest
```

The tests sign with a throwaway local key and check the signatures with
`gpgv`, and with `sqv` (what apt uses from Debian 13 and Ubuntu 25.04) when
it's installed. `--local-key key.pem` in place of `--kms-key` does the same
for trying the whole thing out.
