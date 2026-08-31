//
// The WASM Sound_Core is the SHARED one.  See ../common_sound_core.cpp:
// SDL2 audio + stb_vorbis, identical on Linux, Windows and WASM.  This file is
// a forwarder so each build system's non-recursive OS/WASM/*.cpp glob still
// picks the implementation up, the same way gl30.h includes ../common_gl30.h.
//
#include "sound_core.h"
#include "../common_sound_core.cpp"
