RWK is located in /Games/RWK
If you're using Visual Studio, open /Games/RWK/Project/Win.
The version of Visual Studio I use replaces relative dirs with hard-mapped directories so you will need to adjust the locations of /Framework files.

If you're just using cmake, use it from /Games/RWK/Project/CMakeWin
All the paths here are relative, so invoke CMake from inside this directory.

The resulting exe will end up in /Resources.  This is the folder you zip up to distribute with.

* A note on the source:

So this source goes all the way back to iPhone one.  Because of the memory limits of iPhone 1, you'll find lots of code in here that got depreciated (blank functions and such).  Don't wonder too much about blank functions, mostly they're situations where it used to be a low-weight/low-memory menu system that got replaced.  Also, I took out all the actual server calls to connect to Makermall, but left them with example code, so you can build your own server-side level sharing without too much difficulty.  Just look in Framework/Rapt/rapt_comm.cpp and search for www, and replace those with actual URLs.

* Beepbox

The Beepbox folder contains a Beepbox composer that matches versions with the version baked into RwK.  It can be hosted anywhere.  In WorldEditor.cpp you should look for this line:

    OpenURL("http://www.yoururl.com/compose.html")

...and switch it to wherever you host the Beepbox.html

* Bundling graphics and sounds

    [COMMUNITY 2026] The original note here described a tool called Bundler,
    shipped as prebuilt Linux and Win32 binaries in /Tools, that you dragged a
    folder of source images onto to rebuild an atlas and its .bundle.  Neither
    binary is in this repository: they are prebuilt binaries of code whose
    licence is unsettled, the Linux one needs libbass.so (which its author
    said is not free to distribute) to run at all, and there is no original
    art here for them to bundle.

    What replaces them for THIS repository's purposes is tools/genpack, which
    writes the placeholder pack -- atlases, .bundle files, fonts and sound
    placeholders -- by reading each bundle's layout out of the engine's own
    source.  See tools/genpack/bundle_format.py for the .bundle wire format,
    which is documented there in full.

    If you are working from your own art and want a Bundler of your own, the
    format it has to WRITE is the one documented in
    tools/genpack/bundle_format.py, and the per-folder _bundle.txt /
    _soundbundle.txt files the original tool read are described in its own
    documentation, not here.

* Sound

    [COMMUNITY 2026] BASS is no longer used anywhere.  The original note here
    read: "All my games use BASS for sound and music since I purchased an
    unlimited license...jeez decades ago.  Anyway, I don't think BASS is free
    distribution.  I've included the WASM version of the sound core in
    /Framework/OS/WASM to help you get around this."

    That workaround is no longer needed either.  There is now ONE Sound_Core
    for every platform -- Framework/OS/common_sound_core.cpp -- built on SDL2's
    audio callback (SDL2 was already a dependency everywhere) and the
    public-domain stb_vorbis in Framework/OS/ThirdParty/stb.  The three
    Framework/OS/<Platform>/sound_core.cpp are one-line includes of it.  No
    bass.dll, no libbass.so, no OpenAL and no OpenMPT.

    One capability was dropped with them: TRACKER-MODULE music (LoadMusic /
    PlayMusic / SetMusicTrackVolume) is a logging stub.  RWK never reached it --
    all 61 of its assets are .ogg and its music plays through SoundStream.  A
    game that does want module music has to put a decoder back behind those
    seven functions.

    Tools/libbass.so is gone too, and with it Tools/BundlerLinux, which had
    libbass.so in its ELF NEEDED list and no source in this tree.



