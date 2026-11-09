# Monocypher

A small cryptography library, used by AirPlay output for ChaCha20-Poly1305,
X25519 and Ed25519.

`monocypher.c` and `monocypher.h`, and `monocypher-ed25519.c` and
`monocypher-ed25519.h` from its `src/optional`, are copied from the `src`
directory of Monocypher's 4.0.3 tag, with `__git__` replaced by `4.0.3` as its
release script does for the tarball, and otherwise unmodified:
https://github.com/LoupVaillant/Monocypher/tree/4.0.3/src
(commit ab2b16dd619ad5f6979a4fbe69cfa324a6fcc35f)

`LICENCE.md` is Monocypher's, from the same release: BSD-2-Clause or CC0.
