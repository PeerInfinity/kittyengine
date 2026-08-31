#!/usr/bin/env python3
"""Generate the placeholder asset pack.

The engine in this repository ships with NO original art, sound or level data.
It still has to boot, and a missing asset is not survivable: the loader reads
sprite metadata straight out of a ``.bundle`` with no existence check, so an
absent image file is a silent crash at start-up (the web build aborts on a
malformed one too).  Every file the engine opens therefore has to be present,
and the ``.bundle`` files have to be structurally real.

So this script writes a complete stand-in pack -- one atlas and one bundle per
sprite bundle, a bitmap face per font, a placeholder for every sound, and the
application icon -- entirely from shapes and colours it computes itself.  The
output is original work and is dedicated to the public domain (CC0).

Nothing here is a list of the original's contents.  The layout of every bundle
is READ OUT OF THE ENGINE'S OWN SOURCE at generation time (see
``bundle_format``), the sound names come from the ``sounds://`` references in
the source, and the tile pitch comes from ``World::mGridSize``.  Point the
engine at a different revision and the pack follows it.

    python3 tools/genpack/genpack.py            # write the pack
    python3 tools/genpack/genpack.py --check    # regenerate and diff, write nothing

Requires Pillow.
"""

from __future__ import annotations

import argparse
import colorsys
import hashlib
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import pixelfont                                            # noqa: E402
from bundle_format import BundleWriter, read_layouts        # noqa: E402

try:
    from PIL import Image, ImageDraw
except ImportError:                                         # pragma: no cover
    raise SystemExit("the pack generator needs Pillow:  pip install Pillow")

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
GAME = os.path.join(ROOT, "RWK_Source", "Games", "RWK")
SOURCE = os.path.join(GAME, "Source")
RESOURCES = os.path.join(GAME, "Resources")

#: sprites in the tile bundle draw one grid cell; everything else gets this
DEFAULT_SPRITE_PX = 24
#: a font whose name carries no point size.  Only ``mFont_Tiny`` and the
#: loader's ``mFont_Text`` land here, and both want to be small.
DEFAULT_FONT_POINTS = 10
#: the atlas never grows past this; a bundle needing more is a real change
MAX_ATLAS = 2048
PAD = 1

#: ``000#click.ogg`` sits in the original sound folder as a duplicate of
#: ``click.ogg`` and no code opens it by that name, so a scan of the source
#: cannot find it.  It is generated anyway, for exact filename parity with the
#: layout the engine was shipped with.
UNREFERENCED_SOUNDS = ("000#click",)


# ------------------------------------------------------------------ engine facts
def grid_size() -> int:
    """``World::mGridSize`` -- the tile pitch, in pixels."""
    with open(os.path.join(SOURCE, "World.h"), encoding="utf-8") as fh:
        match = re.search(r"mGridSize\s*=\s*(\d+)", fh.read())
    if not match:
        raise SystemExit("World.h no longer declares mGridSize")
    return int(match.group(1))


def sound_names() -> list:
    """Every ``sounds://NAME`` the engine can ask for."""
    names = set(UNREFERENCED_SOUNDS)
    for base, _dirs, files in os.walk(os.path.join(ROOT, "RWK_Source")):
        for filename in files:
            if not filename.endswith((".cpp", ".h")):
                continue
            with open(os.path.join(base, filename), encoding="utf-8",
                      errors="replace") as fh:
                for name in re.findall(r'sounds://([A-Za-z0-9_#/-]+)', fh.read()):
                    names.add(name)
    # the sound core's own doc comment shows the scheme with a placeholder name
    names.discard("filename")
    return sorted(names)


# ---------------------------------------------------------------------- colours
def _hash(*parts) -> int:
    return int(hashlib.md5("/".join(str(p) for p in parts).encode()).hexdigest(), 16)


def colour(bundle, member, index, spread) -> tuple:
    """A stable colour for one sprite.

    ``spread`` turns the index into a hue rotation instead of a lightness
    step, which is what the tile bundle wants: there, the index IS the layout
    id, and two adjacent ids must not look alike.
    """
    seed = _hash(bundle, member)
    hue = (seed % 3600) / 3600.0
    if spread:
        hue = (hue + index * 0.6180339887) % 1.0
        light = 0.46 + ((seed >> 12) % 5) * 0.03
        sat = 0.55 + ((seed >> 20) % 4) * 0.07
    else:
        light = 0.40 + (index % 5) * 0.045
        sat = 0.42 + ((seed >> 20) % 5) * 0.07
    red, green, blue = colorsys.hls_to_rgb(hue, light, sat)
    return (int(red * 255), int(green * 255), int(blue * 255), 255)


def shade(rgba, factor) -> tuple:
    return tuple(min(255, max(0, int(channel * factor))) for channel in rgba[:3]) + (rgba[3],)


# ----------------------------------------------------------------------- packing
class Atlas:
    """A shelf packer over a square, power-of-two RGBA image."""

    def __init__(self):
        self.boxes = []

    def add(self, width, height):
        self.boxes.append((width, height))
        return len(self.boxes) - 1

    def pack(self):
        """Place every box.  Returns ``(side, [(x, y, w, h), ...])``."""
        side = 64
        while side <= MAX_ATLAS:
            placed, x, y, shelf = [], PAD, PAD, 0
            for width, height in self.boxes:
                if x + width + PAD > side:
                    x, y, shelf = PAD, y + shelf + PAD, 0
                if y + height + PAD > side:
                    placed = None
                    break
                placed.append((x, y, width, height))
                x += width + PAD
                shelf = max(shelf, height)
            if placed is not None:
                return side, placed
            side *= 2
        raise SystemExit("a bundle no longer fits in a %dx%d atlas" % (MAX_ATLAS, MAX_ATLAS))


# ------------------------------------------------------------------------- fonts
def font_points(member: str) -> int:
    digits = re.findall(r"(\d+)", member)
    return int(digits[-1]) if digits else DEFAULT_FONT_POINTS


def font_scale(points: int) -> int:
    return max(1, int(round(points / float(pixelfont.HEIGHT))))


def draw_glyph(image, ox, oy, char, scale, ink, outline):
    draw = ImageDraw.Draw(image)
    rows = pixelfont.rows(char)
    if outline:
        for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            for row, bits in enumerate(rows):
                for col, bit in enumerate(bits):
                    if bit != "#":
                        continue
                    x = ox + (col + dx / float(scale)) * scale
                    y = oy + (row + dy / float(scale)) * scale
                    draw.rectangle([x, y, x + scale - 1, y + scale - 1], fill=(0, 0, 0, 255))
    for row, bits in enumerate(rows):
        for col, bit in enumerate(bits):
            if bit != "#":
                continue
            x, y = ox + col * scale, oy + row * scale
            draw.rectangle([x, y, x + scale - 1, y + scale - 1], fill=ink)


# -------------------------------------------------------------------- generation
def build_bundle(layout, grid):
    """Returns ``(PIL image, bundle bytes)`` for one bundle."""
    is_tiles = layout.name == "Tiles"
    atlas, plan = Atlas(), []

    seen = {}
    for item in layout.items:
        index = seen.get(item.member, 0)
        seen[item.member] = index + 1
        if item.kind == "sprite":
            if item.member == "mFillrect":
                size = 8                    # a solid nib the engine fills rects with
            elif is_tiles:
                size = grid
            else:
                size = DEFAULT_SPRITE_PX
            atlas.add(size, size)
            plan.append(("sprite", item, index, size))
        else:
            points = font_points(item.member)
            scale = font_scale(points)
            width, height = pixelfont.WIDTH * scale, pixelfont.HEIGHT * scale
            codes = pixelfont.codepoints()
            for _code in codes:
                atlas.add(width, height)
            plan.append(("font", item, index, (points, scale, width, height, codes)))

    side, boxes = atlas.pack()
    image = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    writer = BundleWriter()
    cursor = 0

    for kind, item, index, spec in plan:
        texture = 0 if item.texture_char else None
        if kind == "sprite":
            x, y, width, height = boxes[cursor]
            cursor += 1
            if item.member == "mFillrect":
                draw.rectangle([x, y, x + width - 1, y + height - 1],
                               fill=(255, 255, 255, 255))
            else:
                fill = colour(layout.name, item.member, index, spread=is_tiles)
                draw.rectangle([x, y, x + width - 1, y + height - 1], fill=fill)
                draw.rectangle([x, y, x + width - 1, y + height - 1],
                               outline=shade(fill, 0.55))
                draw.line([x + 2, y + 2, x + width - 3, y + height - 3],
                          fill=shade(fill, 1.35))
            writer.sprite(rect=(x, y, width, height), size=(width, height),
                          draw=(width, height), texture=texture)
            continue

        points, scale, width, height, codes = spec
        outline = "Outline" in item.member
        ink = (255, 255, 255, 255)
        glyphs = []
        for code in codes:
            x, y, _w, _h = boxes[cursor]
            cursor += 1
            draw_glyph(image, x, y, chr(code), scale, ink, outline)
            glyphs.append((code, (pixelfont.WIDTH + 1) * scale,
                           (width / 2.0, height / 2.0),
                           dict(rect=(x, y, width, height), size=(width, height),
                                draw=(width, height))))
        writer.font(size=pixelfont.HEIGHT * scale,
                    space_width=(pixelfont.WIDTH - 1) * scale,
                    ascent=pixelfont.HEIGHT * scale,
                    glyphs=glyphs, texture=texture)

    assert cursor == len(boxes), "packed %d boxes, wrote %d" % (len(boxes), cursor)
    return image, writer.bytes()


def app_icon():
    image = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.rounded_rectangle([2, 2, 61, 61], radius=10, fill=(38, 44, 62, 255),
                           outline=(120, 190, 220, 255), width=3)
    draw.rectangle([18, 22, 27, 31], fill=(120, 190, 220, 255))
    draw.rectangle([36, 22, 45, 31], fill=(120, 190, 220, 255))
    draw.rectangle([20, 42, 43, 46], fill=(120, 190, 220, 255))
    return image


def write(path, payload, changed, check):
    old = None
    if os.path.exists(path):
        with open(path, "rb") as fh:
            old = fh.read()
    if old == payload:
        return
    changed.append(os.path.relpath(path, ROOT))
    if check:
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as fh:
        fh.write(payload)


def png_bytes(image):
    import io
    buffer = io.BytesIO()
    image.save(buffer, format="PNG", optimize=False)
    return buffer.getvalue()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true",
                    help="report what regeneration WOULD change and write nothing")
    args = ap.parse_args()

    grid = grid_size()
    layouts = read_layouts(SOURCE)
    changed = []

    for layout in layouts:
        image, blob = build_bundle(layout, grid)
        write(os.path.join(RESOURCES, "images", layout.name + ".png"),
              png_bytes(image), changed, args.check)
        write(os.path.join(RESOURCES, "images", layout.name + ".bundle"),
              blob, changed, args.check)
        print("%-10s %4d sprites %d fonts  atlas %dx%d  bundle %d bytes"
              % (layout.name, len(layout.sprites), len(layout.fonts),
                 image.size[0], image.size[1], len(blob)))

    sounds = sound_names()
    for name in sounds:
        # Measured (N1a): the sound core opens every one of these and survives an
        # empty file with a named warning, on the native build and in the browser
        # alike.  No encoder ships with this repository, and inventing a Vorbis
        # stream to hold silence would add a binary blob nobody can regenerate,
        # so silence here is a zero-length file.
        write(os.path.join(RESOURCES, "sounds", name + ".ogg"), b"", changed, args.check)
    print("%-10s %4d files" % ("sounds", len(sounds)))

    import io
    icon = io.BytesIO()
    app_icon().save(icon, format="ICO", sizes=[(16, 16), (32, 32), (64, 64)])
    write(os.path.join(GAME, "Project", "Win", "AppIcon.ico"), icon.getvalue(),
          changed, args.check)

    if args.check:
        if changed:
            print("\nPACK IS STALE -- regenerating would change %d file(s):" % len(changed))
            for path in changed[:20]:
                print("  " + path)
            if len(changed) > 20:
                print("  ... and %d more" % (len(changed) - 20))
            return 1
        print("\npack is up to date -- regeneration reproduces every file byte for byte")
        return 0

    print("\nwrote/updated %d file(s)" % len(changed))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
