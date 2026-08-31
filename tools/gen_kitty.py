#!/usr/bin/env python3
"""P0" E3: write a SYNTHETIC .kitty from scratch (no campaign bytes copied).

Format per P0' S4(a) + World::Sync at SAVEGAME_VERSION 0x0001:
  int32 version
  chunk = int32 payloadLen, payload, int32 kidCount, kid*
  main chunk: payload 0 B, 6 kids
    [0] meta   : int32 len, name+NUL
    [1] grid   : int32 w, int32 h, w*h * uint32   (no mLevelMap at v1)
    [2] robot  : float x, float y
    [3] kitty  : float x, float y
    [4] extra  : mMessage, mUpMessage (len-prefixed, NUL-terminated),
                 mMessageFade, mSpawnSpot(2f), mFlashWhite, mShake, mZoom,
                 mScrollOffset(2f), mSpawnMusic  -> 35 B + 36 B = 71 B when
                 both strings are the campaign pair; ours are our own.
    [5] tail   : int32
  int32 trailer
Grid word: layout:7 | paint:9 | customDraw:1 | extraData:6 | paintId:9 (LSB first).
Layout 0 = empty, 1 = generic solid wall (World::IsBlocked treats !=0 as solid,
bar the named exceptions).  Tiles are 40 px.
"""
import struct, sys, argparse

TILE = 40

def cell(layout=0, paint=0, custom=0, extra=0, paint_id=0):
    return (layout & 0x7f) | ((paint & 0x1ff) << 7) | ((custom & 1) << 16) \
         | ((extra & 0x3f) << 17) | ((paint_id & 0x1ff) << 23)

def s(text):
    b = text.encode("ascii") + b"\0"
    return struct.pack("<i", len(b)) + b

def chunk(payload, kids=()):
    out = struct.pack("<i", len(payload)) + payload + struct.pack("<i", len(kids))
    for k in kids: out += k
    return out

def leaf(payload): return chunk(payload)

def build(name, w, h, floor_rows=2, platforms=(), robot=None, kitty=None,
          spawn_music=1, zoom=0.9):
    g = [cell(0)] * (w * h)
    def put(x, y, c):
        if 0 <= x < w and 0 <= y < h: g[y * w + x] = c
    solid = cell(1)
    for x in range(w):                      # ceiling + floor
        put(x, 0, solid)
        for r in range(floor_rows): put(x, h - 1 - r, solid)
    for y in range(h):                      # side walls
        put(0, y, solid); put(w - 1, y, solid)
    for (px, py, pw) in platforms:
        for i in range(pw): put(px + i, py, solid)

    if robot is None: robot = (3.0, h - floor_rows - 1.0)
    if kitty is None: kitty = (w - 4.0, h - floor_rows - 1.0)
    rx, ry = robot[0] * TILE, robot[1] * TILE
    kx, ky = kitty[0] * TILE, kitty[1] * TILE

    grid_payload = struct.pack("<ii", w, h) + struct.pack("<%dI" % (w * h), *g)
    extra = (s("") + s("")
             + struct.pack("<f", 0.0)                  # mMessageFade
             + struct.pack("<ff", rx, ry)              # mSpawnSpot
             + struct.pack("<f", 0.0)                  # mFlashWhite
             + struct.pack("<f", 0.0)                  # mShake
             + struct.pack("<f", zoom)                 # mZoom
             + struct.pack("<ff", 0.0, 0.0)            # mScrollOffset
             + struct.pack("<i", spawn_music))
    kids = [leaf(s(name)), leaf(grid_payload),
            leaf(struct.pack("<ff", rx, ry)), leaf(struct.pack("<ff", kx, ky)),
            leaf(extra), leaf(struct.pack("<i", 0))]
    return struct.pack("<i", 1) + chunk(b"", kids) + struct.pack("<i", 0)

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("out"); ap.add_argument("--name", default="P0DTEST")
    ap.add_argument("-W", type=int, default=40); ap.add_argument("-H", type=int, default=20)
    a = ap.parse_args()
    # one platform to jump onto, so a tape can prove collision in a level we authored
    data = build(a.name, a.W, a.H, platforms=[(10, a.H - 5, 6), (20, a.H - 8, 6)])
    open(a.out, "wb").write(data)
    print("wrote %s: %d bytes, %dx%d" % (a.out, len(data), a.W, a.H))
