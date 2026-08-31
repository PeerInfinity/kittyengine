This is a testbed port of Rapt to SDL+OpenGL on windows.
I did it to help me port graphics_core to OpenGL4.

[COMMUNITY 2026] The note that used to be here read: "Currently, sound doesn't
play, I THINK because SDL has taken over the sound card (BASS gives an error
that just says -1, mysterious other error)".

That diagnosis was right, and the conflict is now gone: Sound_Core no longer
uses BASS at all.  Framework/OS/common_sound_core.cpp opens the device through
SDL2 itself -- the same SDL that had "taken over the sound card" -- so there
are no longer two libraries competing for it.  Sound plays.

The REAL downside of this is all the DLLs you have to include (everything in the DDLs directory 
has to go in your distrib folder)



