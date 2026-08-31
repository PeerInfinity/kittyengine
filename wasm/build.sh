#!/usr/bin/env bash
#
# Build kittyengine for the browser with emscripten.
#
# ONE script, on purpose.  This replaces a compile driver and a relink driver
# that had drifted apart -- the link flags in one were missing the SDL_image
# ports the other passed, which the gl11 render path needs -- so the link line
# now exists exactly once, and `--relink` reuses the object cache instead of
# keeping a second copy of it.
#
#   ./wasm/build.sh                 compile what changed, then link
#   ./wasm/build.sh --relink        link only, from the existing objects
#   ./wasm/build.sh --fresh         throw the object cache away first
#
# Environment:
#   EMSDK_ENV   path to an emsdk_env.sh to source.  Not needed when em++ is
#               already on PATH (which is what the CI's emsdk action does).
#   OPT         optimisation level for the compile step (default -O1)
#   JOBS        parallel compiles (default: nproc)
#
set -uo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/.." && pwd)
SRC=${SRC:-$ROOT/RWK_Source}
OBJ=${OBJ:-$HERE/obj}
OUT=${OUT:-$HERE/page}
JOBS=${JOBS:-$(nproc)}
OPT=${OPT:--O1}

RELINK=0; FRESH=0
for arg in "$@"; do
  case "$arg" in
    --relink) RELINK=1 ;;
    --fresh)  FRESH=1 ;;
    *) echo "unknown option: $arg" >&2; exit 2 ;;
  esac
done

if [ -n "${EMSDK_ENV:-}" ]; then
  # shellcheck disable=SC1090
  source "$EMSDK_ENV" >/dev/null 2>&1
fi
if ! command -v em++ >/dev/null 2>&1; then
  echo "em++ is not on PATH.  Install emsdk and either activate it or set" >&2
  echo "EMSDK_ENV=/path/to/emsdk/emsdk_env.sh" >&2
  exit 1
fi

INCS=(
  -I"$SRC/Framework/OS/WASM"
  -I"$SRC/Framework/OS"
  -I"$SRC/Framework/RAPT"
  -I"$SRC/Games/RWK/Source"
)
DEFS=( -DLEGACY_GL -DGL_LEGACY -DRAPT_LEGACY -DNO_RAPT_ML -DRWK_GL11_GLUE )
# the emscripten "ports" the sources' headers need at COMPILE time as well as
# at link time -- SDL_image among them, which is what the two old scripts
# disagreed about
PORTS=( -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=png,jpg
        -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1 -sUSE_ZLIB=1 )
WARN=( -w -fpermissive -Wno-int-to-pointer-cast )
CXXFLAGS=( -std=c++17 $OPT "${DEFS[@]}" "${INCS[@]}" "${PORTS[@]}" "${WARN[@]}" )

[ "$FRESH" = 1 ] && rm -rf "$OBJ"
mkdir -p "$OBJ" "$OUT"

mapfile -t SOURCES < <(
  find "$SRC/Games/RWK/Source" -name '*.cpp' | sort
  ls "$SRC/Framework/OS/WASM"/*.cpp | grep -v /graphics_core.cpp
  # the browser renderer is the Linux GL path behind __EMSCRIPTEN__ guards; the
  # WASM legacy_graphics_core.h path never drew under WebGL
  echo "$SRC/Framework/OS/Linux/graphics_core.cpp"
  ls "$SRC/Framework/RAPT"/*.cpp
  echo "$HERE/src/main_wasm.cpp"
)

if [ "$RELINK" = 0 ]; then
  echo "== compiling ${#SOURCES[@]} translation units at -j$JOBS ($OPT) =="
  export OBJ CXXFLAGS_STR="${CXXFLAGS[*]}" EMSDK_ENV="${EMSDK_ENV:-}"
  printf '%s\n' "${SOURCES[@]}" | xargs -P "$JOBS" -I{} bash -c '
    [ -n "$EMSDK_ENV" ] && source "$EMSDK_ENV" >/dev/null 2>&1
    f={}
    o="$OBJ/$(echo "$f" | md5sum | cut -c1-8)_$(basename "$f" .cpp).o"
    if [ -f "$o" ] && [ "$o" -nt "$f" ]; then exit 0; fi
    if ! em++ $CXXFLAGS_STR -c "$f" -o "$o" 2> "$o.log"; then
      echo "FAIL $f"; rm -f "$o"; exit 0
    fi
  '
  HAVE=$(ls "$OBJ"/*.o 2>/dev/null | wc -l)
  echo "== $HAVE / ${#SOURCES[@]} objects present =="
  if [ "$HAVE" -lt "${#SOURCES[@]}" ]; then
    echo "== compile errors: =="
    for log in "$OBJ"/*.o.log; do
      [ -f "${log%.log}" ] || { echo "--- $log"; sed -n '1,20p' "$log"; }
    done
    exit 1
  fi
fi

#
# What the web build carries: the placeholder pack, the .ml UI markup, and the
# demo levels.  The levels live in samples/ (one folder each) but the engine
# wants them all under data://, so the preload set is staged.
#
RES="$SRC/Games/RWK/Resources"
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/data"
cp "$RES"/data/* "$STAGE/data/" 2>/dev/null
find "$ROOT/samples" -name '*.kitty' -exec cp {} "$STAGE/data/" \;

echo "== linking =="
em++ "$OBJ"/*.o \
  -O1 \
  -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=png,jpg \
  -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1 -sUSE_ZLIB=1 \
  -lidbfs.js -sFETCH=1 \
  -sLEGACY_GL_EMULATION=1 \
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=268435456 -sSTACK_SIZE=5242880 \
  -sASSERTIONS=1 -sEXIT_RUNTIME=0 \
  -sEXPORTED_FUNCTIONS=_main,_IDBFS_ReadSync,_IDBFS_WriteSync,_SetPasteData,_AdComplete,_malloc,_free,_rwk_state,_rwk_pause,_rwk_take_input,_rwk_set_input,_rwk_seed,_rwk_load_level,_rwk_observe,_rwk_step,_rwk_run_tape,_rwk_obs_floats,_rwk_start_persistent,_rwk_persistent_synced,_rwk_list_local_levels,_rwk_makermall_local,_rwk_sandbox_dir,_rwk_key_down,_rwk_sdl_key_raw${EXTRA_EXPORTS:-} \
  -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,FS,IDBFS,stringToNewUTF8,UTF8ToString,HEAPF32,HEAP32,HEAPU8 \
  --preload-file "$STAGE/data@/data" \
  --preload-file "$RES/images@/images" \
  --preload-file "$RES/sounds@/sounds" \
  ${EXTRA_LINK:-} \
  -o "$OUT/index.js" 2>&1 | tail -20

test -f "$OUT/index.wasm" || { echo "LINK FAILED -- no index.wasm"; exit 1; }
ls -la "$OUT"/index.js "$OUT"/index.wasm "$OUT"/index.data
