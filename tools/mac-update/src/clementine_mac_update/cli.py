"""clementine-mac-update: Sparkle for Clementine's macOS build.

clementine-mac-update fetch-sparkle <dir>
clementine-mac-update plan [--notes <file>] [--always]
clementine-mac-update publish --dmg <file> --url <url> --signature <sig>
    --notes <file> [--dry-run]
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path

from clementine_mac_update import plan, publish, sparkle


def _fetch_sparkle(args: argparse.Namespace) -> None:
    sparkle.fetch(args.dir)


def _plan(args: argparse.Namespace) -> None:
    repository = os.environ.get("GITHUB_REPOSITORY", "clementine-player/Clementine")
    update = plan.plan(
        args.repo, args.master, plan.github_dmg(repository), always=args.always
    )
    # key=value lines, for $GITHUB_OUTPUT.
    if update is None:
        print("publish=false")
        return
    args.notes.write_text("".join(f"{note}\n" for note in update.notes))
    print("publish=true")
    print(f"release={update.release}")
    print(f"dmg={update.dmg}")
    print(f"last={update.last or ''}")


def _publish(args: argparse.Namespace) -> None:
    app = publish.read_app(args.dmg)
    publish.check(app, args.dmg, args.signature)
    notes = [line.strip() for line in args.notes.read_text().splitlines()]
    properties = publish.entry(
        app, args.dmg, args.url, args.signature, [n for n in notes if n]
    )
    for name, value in properties.items():
        print(f"{name}: {value}")
    if not args.dry_run:
        print(f"Published {publish.put(properties)}")


def main() -> None:
    parser = argparse.ArgumentParser(prog="clementine-mac-update")
    commands = parser.add_subparsers(required=True)

    fetch = commands.add_parser(
        "fetch-sparkle",
        help="download the pinned Sparkle release, for the build and to sign",
    )
    fetch.add_argument("dir", type=Path)
    fetch.set_defaults(run=_fetch_sparkle)

    planner = commands.add_parser(
        "plan", help="decide whether there's an update to publish, and which"
    )
    planner.add_argument(
        "--notes",
        type=Path,
        default=Path("notes.txt"),
        help="where to write its release notes, one per line",
    )
    planner.add_argument(
        "--always",
        action="store_true",
        help="publish any change since the last update, with or without notes",
    )
    planner.add_argument("--repo", type=Path, default=Path())
    planner.add_argument("--master", default="origin/master")
    planner.set_defaults(run=_plan)

    publisher = commands.add_parser("publish", help="put a signed DMG in the feed")
    publisher.add_argument("--dmg", type=Path, required=True)
    publisher.add_argument("--url", required=True, help="the DMG's download URL")
    publisher.add_argument(
        "--signature", required=True, help="sign_update's EdDSA signature of the DMG"
    )
    publisher.add_argument(
        "--notes", type=Path, required=True, help="release notes, one per line"
    )
    publisher.add_argument(
        "--dry-run",
        action="store_true",
        help="check and print the entry, without publishing it",
    )
    publisher.set_defaults(run=_publish)

    args = parser.parse_args()
    args.run(args)


if __name__ == "__main__":
    main()
