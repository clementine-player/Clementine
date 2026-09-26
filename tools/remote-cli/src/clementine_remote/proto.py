"""Loads the remote's protobuf definitions.

The .proto file lives in the Clementine source tree, so the Python module is
compiled from it on first use (with the protoc bundled in grpcio-tools) and
cached by content hash. Editing the .proto picks up the change on the next run.
"""

from __future__ import annotations

import hashlib
import importlib.util
import os
import sys
from pathlib import Path
from types import ModuleType

PROTO_NAME = "remotecontrolmessages.proto"


def find_proto() -> Path:
    override = os.environ.get("CLEMENTINE_REMOTE_PROTO")
    if override:
        return Path(override)
    # tools/remote-cli/src/clementine_remote/proto.py -> repository root
    root = Path(__file__).resolve().parents[4]
    return root / "ext" / "libclementine-remote" / PROTO_NAME


def _cache_dir() -> Path:
    base = os.environ.get("XDG_CACHE_HOME") or Path.home() / ".cache"
    return Path(base) / "clementine-remote-cli"


def load() -> ModuleType:
    proto = find_proto()
    if not proto.exists():
        raise SystemExit(
            f"Can't find {PROTO_NAME} at {proto}; set CLEMENTINE_REMOTE_PROTO"
        )

    digest = hashlib.sha256(proto.read_bytes()).hexdigest()[:16]
    out_dir = _cache_dir() / digest
    module_path = out_dir / "remotecontrolmessages_pb2.py"

    if not module_path.exists():
        from grpc_tools import protoc

        out_dir.mkdir(parents=True, exist_ok=True)
        result = protoc.main(
            [
                "protoc",
                f"--proto_path={proto.parent}",
                f"--python_out={out_dir}",
                str(proto),
            ]
        )
        if result != 0:
            raise SystemExit(f"protoc failed to compile {proto}")

    spec = importlib.util.spec_from_file_location(
        "remotecontrolmessages_pb2", module_path
    )
    assert spec and spec.loader
    module = importlib.util.module_from_spec(spec)
    sys.modules["remotecontrolmessages_pb2"] = module
    spec.loader.exec_module(module)
    return module


pb = load()
