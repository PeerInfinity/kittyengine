SETLOCAL ENABLEDELAYEDEXPANSION

echo off
set WASM_RESOURCES=--preload-file music@/music 

set COMMON_FILES=
for %%X in (libopenmpt/common/*.cpp) do (
		set aFN=%%~nX
		call set COMMON_FILES=%%COMMON_FILES%% libopenmpt/common/!aFN!.cpp
	)

set SOUNDLIB_FILES=
for %%X in (libopenmpt/soundlib/*.cpp) do (
		set aFN=%%~nX
		call set SOUNDLIB_FILES=%%SOUNDLIB_FILES%% libopenmpt/soundlib/!aFN!.cpp
	)

for %%X in (libopenmpt/soundlib/plugins/*.cpp) do (
		set aFN=%%~nX
		call set SOUNDLIB_FILES=%%SOUNDLIB_FILES%% libopenmpt/soundlib/plugins/!aFN!.cpp
	)

for %%X in (libopenmpt/soundlib/plugins/dmo/*.cpp) do (
		set aFN=%%~nX
		call set SOUNDLIB_FILES=%%SOUNDLIB_FILES%% libopenmpt/soundlib/plugins/dmo/!aFN!.cpp
	)
	
for %%X in (libopenmpt/minimp3/*.cpp) do (
		set aFN=%%~nX
		call set SOUNDLIB_FILES=%%SOUNDLIB_FILES%% libopenmpt/minimp3/!aFN!.cpp
	)
	
	
for %%X in (libopenmpt/soundbase/*.cpp) do (
		set aFN=%%~nX
		call set SOUNDLIB_FILES=%%SOUNDLIB_FILES%% libopenmpt/soundbase/!aFN!.cpp
	)

for %%X in (libopenmpt/sounddsp/*.cpp) do (
		set aFN=%%~nX
		call set SOUNDLIB_FILES=%%SOUNDLIB_FILES%% libopenmpt/sounddsp/!aFN!.cpp
	)
	

set MPT_FILES=
rem for %%X in (libopenmpt/*.cpp) do (
rem 	set aFN=%%~nX
rem  	call set MPT_FILES=%%MPT_FILES%% libopenmpt/!aFN!.cpp
rem 	)

set MPT_FILES=%MPT_FILES% libopenmpt/libopenmpt_c.cpp
set MPT_FILES=%MPT_FILES% libopenmpt/libopenmpt_cxx.cpp
set MPT_FILES=%MPT_FILES% libopenmpt/libopenmpt_impl.cpp
set MPT_FILES=%MPT_FILES% libopenmpt/libopenmpt_ext_impl.cpp


set MY_DEFINES=-DMPT_WITH_SDL2 -DMPT_WITH_OGG -DMPT_WITH_MINIMP3 -DHAVE_ALLOCA_H -DMPT_WITH_VORBIS -DMPT_WITH_VORBISFILE -DNO_XMPLAY -DNO_WINAMP -DLIBOPENMPT_BUILD


echo on
emcc -o ..\openmpt.o -shared --std=c++17 -s USE_OGG=1 %MY_DEFINES% -s USE_ZLIB -s USE_VORBIS=1  -I"soundlib" -I"include" -I"libopenmpt" -I"libopenmpt/common" %COMMON_FILES% %SOUNDLIB_FILES% %MPT_FILES% %WASM_RESOURCES%



