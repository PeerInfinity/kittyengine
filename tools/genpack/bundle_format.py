#!/usr/bin/env python3
"""Read a bundle's LAYOUT out of the engine's own source, and write one.

A ``.bundle`` is sprite metadata: for each sprite, where it sits in the atlas
and how big it draws.  The file carries no names, no count and no header -- it
is a bare stream, and the *only* thing that says what is in it is the order of
the calls in the generated "Bundler Automatic Code" block of the matching
``Bundle_*.cpp``.  Read that block and you have the layout; write the same
stream back and the engine loads it.

So this module does not hardcode a sprite list.  It parses:

  * ``Games/RWK/Source/Bundle_*.cpp`` -- the ordered load calls, including the
    ``GuaranteeSize(N); for (...)`` array forms;
  * every header in the same directory (and ``Dialog.h``, which is where the
    dialog bundles inherit their widgets from) -- the declared type of each
    member, which is what says whether a call reads a Sprite or a whole Font.

The two call spellings differ by one byte:

  ``Sprite::ManualLoad(tex, buffer)``     reads the sprite payload
  ``Sprite::BundleLoad(0, list, buffer)`` reads a texture index CHAR first

and the same split applies to ``Font::ManualLoad`` / ``Font::BundleLoad``,
whose glyph sprites inherit whichever spelling the font was loaded with.

Sprite payload (``rapt_sprite.cpp``, ``Sprite::ManualLoad``)::

    Rect   x, y, w, h        4 x float32   position in the atlas, in pixels
    int    width, height     2 x int32     the sprite's pixel size
    float  drawW, drawH      2 x float32   the size it draws at
    float  xMove, yMove      2 x float32   draw offset from its centre
    bool   rotated           1 x uint8     atlas entry is turned 90 degrees
    int    keyCount          1 x int32     attachment points that follow
    Point  key[keyCount]     2 x float32 each

Font payload (``rapt_font.cpp``, ``Font::ManualLoad``)::

    float  size, spaceWidth, ascent
    kerns:  (int16 c1, int16 c2, float amount)*  terminated by c1==0 && c2==0
    chars:  int16 code (0 ends), float width, Point offset, <sprite payload>

Everything is little-endian: the engine ``memcpy``s these straight out of the
buffer, so the file is in the host's byte order and every platform it has ever
built for is little-endian.
"""

from __future__ import annotations

import os
import re
import struct

#: one sprite entry with no keys, for a size assertion the tests can make
SPRITE_BYTES = 4 * 4 + 4 + 4 + 4 * 4 + 1 + 4

_BLOCK = re.compile(r"// Begin Bundler Automatic Code(.*?)// End Bundler Automatic Code",
                    re.S)
_CALL = re.compile(r"\b(m\w+)((?:\[[^\]]*\])*)\.(ManualLoad|BundleLoad)\s*\(")
_LOOP = re.compile(r"for\s*\(\s*aSCount\s*=\s*0\s*;\s*aSCount\s*<\s*(\d+)\s*;")
_BUNDLE_NAME = re.compile(r'SpriteBundle::Load\("([A-Za-z0-9_]+)"\)')
_DECL = re.compile(r"^\s*(?:Array<\s*)*(Sprite|Font)(?:\s*>)*\s+(m\w+)\s*;", re.M)


class Item:
    """One load call in a bundle's stream."""

    def __init__(self, member, kind, texture_char):
        self.member = member
        self.kind = kind                    # "sprite" or "font"
        self.texture_char = texture_char    # BundleLoad spelling reads one

    def __repr__(self):
        return "Item(%s,%s,%s)" % (self.member, self.kind, self.texture_char)


class Layout:
    """A bundle: its texture name and the ordered stream its .cpp reads."""

    def __init__(self, name, source, items):
        self.name = name
        self.source = source
        self.items = items

    @property
    def sprites(self):
        return [i for i in self.items if i.kind == "sprite"]

    @property
    def fonts(self):
        return [i for i in self.items if i.kind == "font"]


def member_types(source_dir: str) -> dict:
    """member name -> "sprite" | "font", from every header in the game source."""
    types = {}
    for filename in sorted(os.listdir(source_dir)):
        if not filename.endswith(".h"):
            continue
        with open(os.path.join(source_dir, filename), encoding="utf-8",
                  errors="replace") as fh:
            for kind, member in _DECL.findall(fh.read()):
                types.setdefault(member, kind.lower())
    return types


def read_layout(cpp_path: str, types: dict) -> Layout:
    with open(cpp_path, encoding="utf-8", errors="replace") as fh:
        text = fh.read()

    names = _BUNDLE_NAME.findall(text)
    # every bundle loads its texture twice, once for the @2X retina name
    plain = [n for n in names if not n.endswith("@2X")]
    if len(set(plain)) != 1:
        raise AssertionError("%s names %r textures, expected exactly one"
                             % (cpp_path, sorted(set(plain))))

    block = _BLOCK.search(text)
    if not block:
        raise AssertionError("%s has no Bundler Automatic Code block" % cpp_path)

    items = []
    for line in block.group(1).splitlines():
        calls = _CALL.findall(line)
        if not calls:
            continue
        loop = _LOOP.search(line)
        repeat = int(loop.group(1)) if loop else 1
        if len(calls) != 1:
            raise AssertionError("%s: %d load calls on one line, which the "
                                 "repeat count cannot be attributed to: %s"
                                 % (cpp_path, len(calls), line.strip()))
        member, _subscripts, spelling = calls[0]
        if member not in types:
            raise AssertionError("%s: %s is loaded but no header declares it"
                                 % (cpp_path, member))
        for _ in range(repeat):
            items.append(Item(member, types[member], spelling == "BundleLoad"))

    return Layout(plain[0], cpp_path, items)


def read_layouts(source_dir: str) -> list:
    """Every Bundle_*.cpp in ``source_dir`` that actually loads a texture."""
    types = member_types(source_dir)
    out = []
    for filename in sorted(os.listdir(source_dir)):
        if not (filename.startswith("Bundle_") and filename.endswith(".cpp")):
            continue
        path = os.path.join(source_dir, filename)
        with open(path, encoding="utf-8", errors="replace") as fh:
            if "SpriteBundle::Load(" not in fh.read():
                continue        # Bundle_Sounds loads .ogg files, not an atlas
        out.append(read_layout(path, types))
    return out


# --------------------------------------------------------------------- writing
class BundleWriter:
    """Builds the byte stream a ``.bundle`` is."""

    def __init__(self):
        self._parts = []

    def _put(self, fmt, *values):
        self._parts.append(struct.pack("<" + fmt, *values))

    def sprite(self, rect, size, draw, move=(0.0, 0.0), rotated=False,
               keys=(), texture=None):
        """One sprite payload.  ``texture`` prefixes the BundleLoad index char."""
        if texture is not None:
            self._put("b", texture)
        self._put("4f", *[float(v) for v in rect])
        self._put("2i", int(size[0]), int(size[1]))
        self._put("4f", float(draw[0]), float(draw[1]), float(move[0]), float(move[1]))
        self._put("?", bool(rotated))
        self._put("i", len(keys))
        for key in keys:
            self._put("2f", float(key[0]), float(key[1]))

    def font(self, size, space_width, ascent, glyphs, texture=None):
        """One font payload.  ``glyphs`` is ``(codepoint, width, offset, sprite)``.

        No kerning pairs are written: the terminator goes in immediately, which
        is a font with an empty kern table, not a malformed one.
        """
        self._put("3f", float(size), float(space_width), float(ascent))
        self._put("2h", 0, 0)                       # end of the kern table
        for code, width, offset, sprite in glyphs:
            self._put("h", int(code))
            self._put("f", float(width))
            self._put("2f", float(offset[0]), float(offset[1]))
            self.sprite(texture=texture, **sprite)
        self._put("h", 0)                           # end of the character table

    def bytes(self) -> bytes:
        return b"".join(self._parts)


if __name__ == "__main__":
    import sys
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.normpath(os.path.join(here, "..", ".."))
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        root, "RWK_Source", "Games", "RWK", "Source")
    for layout in read_layouts(src):
        print("%-10s %4d sprites  %d fonts  (%s)"
              % (layout.name, len(layout.sprites), len(layout.fonts),
                 "BundleLoad" if layout.items[0].texture_char else "ManualLoad"))
