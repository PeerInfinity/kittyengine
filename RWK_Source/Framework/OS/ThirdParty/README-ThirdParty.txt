Framework/OS/ThirdParty
=======================

Third-party code shared by ALL platform layers (as opposed to the per-platform
ThirdParty directories under OS/Linux, OS/Windows, OS/WASM).

stb/stb_vorbis.c
----------------
  Ogg Vorbis decoder, v1.22, by Sean Barrett (nothings.org/stb_vorbis).
  Fetched verbatim from https://raw.githubusercontent.com/nothings/stb/master/stb_vorbis.c
  md5 36713ac98e445271e29547cc2d90b01f, 5584 lines. NOT modified.

  Dual-licensed; the full text is in the footer of stb_vorbis.c itself:
    ALTERNATIVE A - MIT License, Copyright (c) 2017 Sean Barrett
    ALTERNATIVE B - Public Domain (www.unlicense.org)

  Used by Framework/OS/common_sound_core.cpp, which decodes every .ogg the
  game loads.  It replaces:
    - BASS (proprietary, not free to redistribute) on Linux and Windows;
    - libvorbis/vorbisfile + libopenmpt + OpenAL on WASM.
