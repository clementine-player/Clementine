"""The remote's protobuf definitions, as `pb`.

The .proto file lives in the Clementine source tree, so the Python module and
its type stubs are compiled from it (with the protoc bundled in grpcio-tools)
into the _generated package next to this file, which isn't checked in. They
are rebuilt whenever the .proto changes. Run this module to generate them
without doing anything else, for example before type checking.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path

PROTO_NAME = "remotecontrolmessages.proto"
GENERATED = Path(__file__).parent / "_generated"
# Records which .proto the generated files came from.
STAMP = GENERATED / "SOURCE_SHA256"


def find_proto() -> Path:
    override = os.environ.get("CLEMENTINE_REMOTE_PROTO")
    if override:
        return Path(override)
    # tools/remote-cli/src/clementine_remote/proto.py -> repository root
    root = Path(__file__).resolve().parents[4]
    return root / "ext" / "libclementine-remote" / PROTO_NAME


def generate() -> None:
    """Compiles the .proto into _generated, unless it's already up to date."""
    proto = find_proto()
    if not proto.exists():
        if STAMP.exists():
            return  # Installed away from the source tree; use what's there.
        raise SystemExit(
            f"Can't find {PROTO_NAME} at {proto}; set CLEMENTINE_REMOTE_PROTO"
        )

    digest = hashlib.sha256(proto.read_bytes()).hexdigest()
    if STAMP.exists() and STAMP.read_text().strip() == digest:
        return

    from grpc_tools import protoc

    GENERATED.mkdir(exist_ok=True)
    result = protoc.main(
        [
            "protoc",
            f"--proto_path={proto.parent}",
            f"--python_out={GENERATED}",
            f"--pyi_out={GENERATED}",
            str(proto),
        ]
    )
    if result != 0:
        raise SystemExit(f"protoc failed to compile {proto}")
    (GENERATED / "__init__.py").touch()
    STAMP.write_text(digest + "\n")


generate()

from ._generated import remotecontrolmessages_pb2 as pb  # noqa: E402

__all__ = ["pb"]

if __name__ == "__main__":
    print(f"Generated {GENERATED}")
