#!/usr/bin/env python3
"""Every demo level is COMPLETABLE -- proven in this engine, not asserted.

The four levels in ``samples/`` each ship with an input tape that solves them
and a recorded observation digest.  This gate builds nothing: it runs the engine
you built in its headless ``--oracle`` mode, replays each tape, and requires

  * the engine's own **win flag** (the last column of the observation stream) to
    go true.  ``World::Win()`` is reached from exactly one place in the game --
    ``Kitty::Update``, when the robot is within 35 px -- so the flag really is
    "the robot reached the kitty";
  * the robot never to die on the way, because a walkthrough that dies proves
    the level is survivable, not that the intended solution works;
  * the world to have a real size.  An absent or empty level file does not
    fail: the engine boots a **0x0 world and raises the win flag at tick 0**.
    That reads as an instant, perfect win, so a gate that only looks at the flag
    passes hardest exactly when the levels have gone missing;
  * the tick count to be sane -- a win must arrive after the first tick and
    within the budget the manifest allows;
  * the observation digest to match the recorded one, which turns the tapes into
    a regression gate on the whole chain: level file, container, engine physics.

    python3 scripts/completability_gate.py
    python3 scripts/completability_gate.py --write        # re-record the digests
    python3 scripts/completability_gate.py --self-test    # prove it can go red
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
GAME = os.path.join(ROOT, "RWK_Source", "Games", "RWK")
SAMPLES = os.path.join(ROOT, "samples")
DEFAULT_ENGINE = os.path.join(GAME, "Resources", "kittyengine")

ROBOT_LINE = re.compile(r"world (\d+)x(\d+), robot at \(([-\d.]+),([-\d.]+)\)")

#: the observation stream's columns, as the engine's own header names them
WIN, DIED, TICK, X = -1, -2, 0, 2


def sandbox_folder() -> str:
    """Where the engine keeps its settings, derived from the name it registers."""
    with open(os.path.join(GAME, "Source", "MyApp.cpp"), encoding="utf-8",
              errors="replace") as fh:
        match = re.search(r'SetAppName\("([^"]+)"\)', fh.read())
    if not match:
        raise SystemExit("MyApp.cpp no longer calls SetAppName")
    return os.path.expanduser("~/.local/share/.raptisoft/%s/_sandbox" % match.group(1))


def run_oracle(engine, level, tape, ticks, out_csv, seed):
    """One headless run.  Returns ``(stderr, observation rows)``."""
    # The engine crashes on a settings file a previous run left behind, so the
    # sandbox settings go before every run -- this is a fresh-boot measurement.
    settings = os.path.join(sandbox_folder(), "settings.txt")
    if os.path.exists(settings):
        os.remove(settings)
    cmd = [engine, "--oracle", "--level=" + level,
           "--ticks=%d" % ticks, "--out=" + out_csv, "--seed=%d" % seed]
    if tape:
        cmd.append("--tape=" + tape)
    env = dict(os.environ, SDL_VIDEODRIVER="offscreen", SDL_AUDIODRIVER="dummy")
    proc = subprocess.run(cmd, cwd=os.path.dirname(os.path.abspath(engine)),
                          env=env, capture_output=True, text=True, timeout=900)
    if proc.returncode != 0:
        raise SystemExit("the engine exited %d\n%s" % (proc.returncode, proc.stderr))
    with open(out_csv, encoding="utf-8") as fh:
        rows = [line for line in fh if line and not line.startswith("#")]
    return proc.stderr, rows


def replay(engine, samples, entry, tmp, seed) -> dict:
    name = entry["name"]
    level = os.path.join(samples, name, name + ".kitty")
    tape = os.path.join(samples, name, name + ".tape.csv")
    out = os.path.join(tmp, name + ".obs.csv")
    stderr, rows = run_oracle(engine, level, tape, entry["ticks"], out, seed)

    match = ROBOT_LINE.search(stderr)
    if not match:
        raise SystemExit("%s: could not read the engine's load line:\n%s" % (name, stderr))
    grid = [int(match.group(1)), int(match.group(2))]

    data = [r.rstrip("\n").split(",") for r in rows if not r.startswith(("#", "tick"))]
    won_at = next((int(r[TICK]) for r in data if r[WIN] == "1"), None)
    died_at = next((int(r[TICK]) for r in data if r[DIED] == "1"), None)
    return {
        "name": name,
        "grid": grid,
        "ticks_run": len(data),
        "won_at": won_at,
        "died_at": died_at,
        "final_x": float(data[-1][X]) if data else None,
        # the stream up to and including the winning tick: the ticks AFTER a win
        # belong to the win transition tearing the world down, not to the level
        "digest": hashlib.md5(
            "".join(rows[: (won_at + 3) if won_at is not None else len(rows)]).encode()
        ).hexdigest(),
    }


def judge(entry, got, expected, write) -> list:
    """Every reason this sample fails."""
    name, failures = got["name"], []

    if got["grid"][0] <= 0 or got["grid"][1] <= 0:
        failures.append("%s: the engine loaded a %dx%d world -- an absent or empty "
                        "level file boots a degenerate world that wins at tick 0, "
                        "so this is a missing level, not a completed one"
                        % (name, got["grid"][0], got["grid"][1]))
    elif got["grid"] != entry["grid"]:
        failures.append("%s: the engine loaded %s, the manifest says %s"
                        % (name, got["grid"], entry["grid"]))

    if got["won_at"] is None:
        failures.append("%s: the win flag never went true in %d ticks -- the tape "
                        "does not solve the level" % (name, entry["ticks"]))
    elif got["won_at"] < 1:
        failures.append("%s: the win flag was already true at tick %d, before the "
                        "tape could have done anything"
                        % (name, got["won_at"]))
    elif got["won_at"] > entry["ticks"]:
        failures.append("%s: won at tick %d, past the manifest's %d-tick budget"
                        % (name, got["won_at"], entry["ticks"]))

    if got["died_at"] is not None:
        failures.append("%s: the robot died at tick %d; the intended solution must "
                        "not need a death" % (name, got["died_at"]))

    if write:
        return failures
    if expected is None:
        failures.append("%s has no recorded expectation; run with --write" % name)
    elif expected.get("digest") != got["digest"]:
        failures.append("%s: observation digest %s, recorded %s -- something in the "
                        "chain (level file, container, engine) moved"
                        % (name, got["digest"], expected.get("digest")))
    elif expected.get("won_at") != got["won_at"]:
        failures.append("%s: won at tick %s, recorded %s"
                        % (name, got["won_at"], expected.get("won_at")))
    return failures


def self_test() -> int:
    """The made-to-go-red control: the gate must reject a degenerate world.

    An empty ``.kitty`` is the exact shape of the failure that looks like a win
    -- the engine boots a 0x0 world with the win flag already up.  A gate that
    passes here would pass on a repository that shipped no levels at all.
    """
    entry = {"name": "planted", "grid": [12, 6], "ticks": 60}
    got = {"name": "planted", "grid": [0, 0], "ticks_run": 32, "won_at": 0,
           "died_at": None, "final_x": 0.0, "digest": "0" * 32}
    failures = judge(entry, got, {"digest": "0" * 32, "won_at": 0}, write=False)
    if not any("0x0 world" in f for f in failures):
        print("SELF-TEST FAILED: a 0x0 world with win=1 at tick 0 was accepted")
        return 1
    if not any("before the tape" in f for f in failures):
        print("SELF-TEST FAILED: a win at tick 0 was accepted")
        return 1
    good = {"name": "planted", "grid": [12, 6], "ticks_run": 40, "won_at": 30,
            "died_at": None, "final_x": 1.0, "digest": "0" * 32}
    if judge(entry, good, {"digest": "0" * 32, "won_at": 30}, write=False):
        print("SELF-TEST FAILED: a real win was rejected")
        return 1
    print("self-test PASSED -- a 0x0 instant win is rejected, a real win is not")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--engine", default=DEFAULT_ENGINE)
    ap.add_argument("--samples", default=SAMPLES,
                    help="the sample tree to replay (a MUTANT copy proves the gate "
                         "discriminates)")
    ap.add_argument("--seed", type=int, default=1,
                    help="the engine PRNG seed the recorded digests were taken at")
    ap.add_argument("--write", action="store_true",
                    help="record the results as the expected ones")
    ap.add_argument("--self-test", action="store_true",
                    help="prove the gate rejects a degenerate win, then exit")
    ap.add_argument("--only", action="append")
    args = ap.parse_args()

    if args.self_test:
        return self_test()

    if not os.path.exists(args.engine):
        raise SystemExit("missing the engine build: %s\n"
                         "build it first (see README: Building)" % args.engine)

    with open(os.path.join(args.samples, "samples.json"), encoding="utf-8") as fh:
        manifest = json.load(fh)["samples"]
    expected_path = os.path.join(args.samples, "oracle-expected.json")
    expected = {}
    if os.path.exists(expected_path):
        with open(expected_path, encoding="utf-8") as fh:
            expected = json.load(fh).get("samples", {})

    entries = [e for e in manifest if not args.only or e["name"] in args.only]
    failures, results = [], {}
    with tempfile.TemporaryDirectory() as tmp:
        for entry in entries:
            got = replay(args.engine, args.samples, entry, tmp, args.seed)
            results[got["name"]] = got
            print("%-13s %3dx%-3d  won_at=%-6s died_at=%-6s final_x=%-9s %s"
                  % (got["name"], got["grid"][0], got["grid"][1], got["won_at"],
                     got["died_at"], got["final_x"], got["digest"]))
            failures += judge(entry, got, expected.get(got["name"]), args.write)

    if args.write:
        with open(expected_path, "w", encoding="utf-8") as fh:
            json.dump({
                "_doc": "Expected oracle results for samples/*, recorded by "
                        "scripts/completability_gate.py --write.",
                "_engine": os.path.basename(args.engine),
                "_seed": args.seed,
                "samples": results,
            }, fh, indent=1)
            fh.write("\n")
        print("\nrecorded %d sample(s) in %s" % (len(results), expected_path))
        if failures:
            print("...but the run itself was not clean:")
            for line in failures:
                print("  - " + line)
            return 1
        return 0

    if failures:
        print("\nCOMPLETABILITY FAILED:")
        for line in failures:
            print("  - " + line)
        return 1
    print("\nCOMPLETABILITY PASSED -- every sample's tape reaches the kitty")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
