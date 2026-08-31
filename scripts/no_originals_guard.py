#!/usr/bin/env python3
"""The no-originals guard.

This repository publishes an engine whose original art, music, fonts and levels
belong to someone else.  Not shipping them is a promise, and a promise a
repository can break by accident -- a stray copy under a scratch directory, a
fixture someone found convenient, an asset pulled in by a build step and left
behind in an artifact.  So the promise is a test.

``scripts/known-original-md5s.json`` holds the md5 of every original asset blob
in the full history of the source mirror this engine came from, plus the level
gifs of the game compilation: 1026 hashes.  A hash identifies a file without
carrying any of it, which is why the list itself is publishable.

The guard walks a tree and fails if any file hashes to any of them.  It runs
over the repository in CI, and it takes a path so it can also be pointed at a
BUILT ARTIFACT -- the wasm ``.data`` package, a published site directory, a
fresh clone -- checking what actually went out rather than what was meant to::

    python3 scripts/no_originals_guard.py                 # this repository
    python3 scripts/no_originals_guard.py wasm/page       # a build directory
    python3 scripts/no_originals_guard.py --self-test     # prove it can go red

``--self-test`` is the made-to-go-red control: it plants a file, asks the
scanner for that file's hash, and requires the scanner to find it.  Planting a
file whose md5 is one of the LISTED ones is impossible without the original --
which is the point -- so discrimination is proven the other way round.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
LIST = os.path.join(HERE, "known-original-md5s.json")

#: directories a walk never needs to descend into.  ``.git`` is excluded because
#: its object store holds COMPRESSED blobs, which never match a plain md5 -- the
#: working tree is where an original would actually be readable.
SKIP_DIRS = {".git", "__pycache__", ".pytest_cache", ".venv", "venv",
             "node_modules", "obj", "build"}

#: a web build's preload package CONCATENATES its inputs, so an original inside
#: ``index.data`` is not a whole-file md5 hit -- scanning the artifact naively
#: would pass while shipping every asset.  ``unpack`` splits such a payload back
#: into its members using the offsets emscripten records in the sibling ``.js``.
PACKED_SUFFIXES = (".data",)
PACKED_ENTRY = re.compile(
    r'\{"filename":\s*"([^"]+)",\s*"start":\s*(\d+),\s*"end":\s*(\d+)\}')


def known_md5s(path: str = LIST) -> set:
    with open(path, encoding="utf-8") as fh:
        payload = json.load(fh)
    got = set(payload["md5"])
    if len(got) != payload["_count"]:
        raise SystemExit("the list claims %d hashes and holds %d"
                         % (payload["_count"], len(got)))
    return got


def unpack(path: str):
    """Members of a preload package, as ``(name, bytes)``.

    Returns ``None`` when ``path`` is not one -- no sibling ``.js``, or no
    offset table in it -- so the caller can fall back to hashing it whole.
    """
    sidecar = os.path.splitext(path)[0] + ".js"
    if not os.path.isfile(sidecar):
        return None
    with open(sidecar, encoding="utf-8", errors="replace") as fh:
        entries = PACKED_ENTRY.findall(fh.read())
    if not entries:
        return None
    with open(path, "rb") as fh:
        blob = fh.read()
    return [(name, blob[int(a):int(b)]) for name, a, b in entries]


def scan(root: str, wanted: set) -> list:
    """Every file under ``root`` whose md5 is in ``wanted``."""
    hits = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for filename in filenames:
            path = os.path.join(dirpath, filename)
            if os.path.islink(path) or not os.path.isfile(path):
                continue
            rel = os.path.relpath(path, root)
            try:
                if filename.endswith(PACKED_SUFFIXES):
                    members = unpack(path)
                    if members is not None:
                        for name, body in members:
                            digest = hashlib.md5(body).hexdigest()
                            if digest in wanted:
                                hits.append(("%s!%s" % (rel, name), digest))
                        continue
                with open(path, "rb") as fh:
                    digest = hashlib.md5(fh.read()).hexdigest()
            except OSError:
                continue
            if digest in wanted:
                hits.append((rel, digest))
    return hits


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("target", nargs="?", default=ROOT,
                    help="the tree to scan (default: this repository)")
    ap.add_argument("--list", default=LIST)
    ap.add_argument("--self-test", action="store_true",
                    help="prove the scanner can find a planted file, then exit")
    args = ap.parse_args()

    if args.self_test:
        body = b"a planted file, standing in for an original the guard must find\n"
        with tempfile.TemporaryDirectory() as tmp:
            os.mkdir(os.path.join(tmp, "sub"))
            with open(os.path.join(tmp, "sub", "planted.bin"), "wb") as fh:
                fh.write(body)
            with open(os.path.join(tmp, "innocent.txt"), "wb") as fh:
                fh.write(b"nothing to see\n")
            digest = hashlib.md5(body).hexdigest()
            found = scan(tmp, {digest})
            if found != [(os.path.join("sub", "planted.bin"), digest)]:
                print("SELF-TEST FAILED: the scanner did not find the planted file: %r"
                      % (found,))
                return 1
            if scan(tmp, {"0" * 32}) != []:
                print("SELF-TEST FAILED: the scanner reported a hash nothing has")
                return 1
        print("self-test PASSED -- the scanner finds a planted file and only that file")
        return 0

    wanted = known_md5s(args.list)
    hits = scan(args.target, wanted)
    if hits:
        print("NO-ORIGINALS GUARD FAILED -- original asset data in %s:"
              % os.path.abspath(args.target))
        for path, digest in hits:
            print("  %s  %s" % (digest, path))
        return 1
    print("no-originals guard PASSED over %s (%d hashes)"
          % (os.path.abspath(args.target), len(wanted)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
