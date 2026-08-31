# kittyengine — Windows Build Guide

This guide documents the **minimal, repeatable** steps to build the Windows executable from source using **CLion** and **Visual Studio Build Tools (MSVC)**.

You will end up with:

- `RWK.exe` built into: `C:\RWK_Source\RWK_Source/Games/RWK/Resources/`
- Ready to run from that folder (where the assets + DLLs live)

---

## 0) Folder layout assumed

This guide assumes the extracted the repo like this (top-level example):

```
C:\RWK_Source\RWK_Source\
  Framework\
  Games\RWK\
    Project\CMakeWin\CMakeLists.txt
    Resources\
    Source\
```

The CMake file we use lives here:

```
C:\RWK_Source\RWK_Source\Games\RWK\Project\CMakeWin\CMakeLists.txt
```

---

## 1) Install prerequisites (minimal)

### 1.1 Install CLion
Install JetBrains CLion. https://www.jetbrains.com/clion/download
[example file: CLion-2025.2.5.exe]

```
CLion Pro TIP: Go to Settings --> Editor --> General --> Editor Tabs and check the option "Mark modified"
```

### 1.2 Install Visual Studio Build Tools (MSVC)
Install **Build Tools for Visual Studio 2022** (not the full VS IDE). https://visualstudio.microsoft.com/downloads/
[example file: vs_BuildTools.exe]

Scroll down to "Tools for Visual Studio" and Download the "Build Tools for Visual Studio".

In the installer, only select:
- **Desktop development with C++**

Make sure the installer includes:
- MSVC v143 - VS 2022 C++..
- The latest Windows SDK (it usually installs automatically)

---

## 2) Prepare runtime DLLs (required to run)

### 2.1 Copy bundled RWK Windows DLLs into `Resources`
Copy everything from:

```
C:\RWK_Source\RWK_Source\Framework\OS\Windows\DLLs\
```

into:

```
C:\RWK_Source\RWK_Source\Games\RWK\Resources\
```

### 2.2 ~~Add BASS runtime DLL~~ — NO LONGER NEEDED

**There is nothing to do in this step any more.** BASS has been removed from
the engine: `Sound_Core` is now one shared SDL2 + stb_vorbis implementation
(`Framework\OS\common_sound_core.cpp`) on every platform, and `SDL2.dll` is
already among the DLLs copied in step 2.1.

This step used to say: download BASS Win32 (32-bit) from https://www.un4seen.com/
and copy `bass.dll` into `Resources`, because the repo shipped `bass.lib` and
`bass.h` under `Framework\OS\Windows\BASS\` but not the redistributable DLL,
which was excluded for licensing reasons.

`Framework\OS\Windows\BASS\` and its `bass.lib`/`bass.h` are gone, and the
CMake link line no longer names them. If you are building an older checkout,
follow the original instructions above.

---

## 3) Open the project in CLion

From Windows Explorer, open the 'RWK_Source' folder as a CLion project 

```
C:\RWK_Source\RWK_Source
```

**right-click and choose 'Open Folder as CLion Project'**

Trust the project folder and add to Windows Defender exclusions list

Close the 'Open Project Wizard' window

Open CMakeLists.txt in CLion <double-click>...

```
C:\RWK_Source\RWK_Source\Games\RWK\Project\CMakeWin\CMakeLists.txt
```

You'll see CLion give you a warning at the top of the file "Project is not configured". Click the "Fix..." link to the right.

```
Configure CMake Project --> Select CMakeLists.txt and select the file from the location above.
```

---

## 4) Configure CLion to build **x86** (important)

The shipped libraries in this repo (SDL2, GLEW) are **32-bit (x86)**, so the RWK executable must also be built as **x86**. (BASS — whose `bass.lib` was 32-bit-only and was the hardest constraint here — has been removed; SDL2 and GLEW are what pin the architecture now.)

In CLion:

**Settings → Build, Execution, Deployment → Toolchains**
- Add (+) a new toolchain - 'Visual Studio'
- Toolset: The version we installed (C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools)...ignore the warning
- **Architecture: x86**
- Platform: **leave blank** (do **not** select store/uwp)
- Click Apply

**Settings → Build, Execution, Deployment → CMake**
- Add (+) a new profile and enable it. Move it up the list and disable the default profile.
- Name: Something like "Debug-VisualStudio-86"
- Build type: Debug or Release. Maybe Debug at first for more verbose logging
- Toolchain: use 'Visual Studio'
- Generator: Use default "NMake Makefiles"
- Click Apply and OK

> If you build x64 you’ll see linker warnings/errors like:
> `library machine type 'x86' conflicts with target machine type 'x64'`.

---

## 5) Maintenance
Delete MagicAd.cpp and the related code -> It's not needed and causes errors.

In CLion, press ctrl+shift+F and search for MagicAd. Delete all traces and lines defining/including it.

```
MagicAd.cpp
MagicAd.h
```
Delete entire lines like this

```
<ClInclude Include="..\..\..\Source\MagicAd.h" />
```
Delete entire blocks like this

```
    <ClInclude Include="..\..\..\Source\MagicAd.h">
      <Filter>Header Files</Filter>
    </ClInclude>
```
---

## 6) Define the LEGACY
Edit C:\RWK_Source\RWK_Source\Framework\RAPT\rapt_CPU.h

Include the following code where you see the '//(COMMUNITY_2026)' comment

```
#pragma once
#include "rapt_object.h"
#include "rapt_rect.h"
#include "rapt_string.h"

#define CPUHOOK(func) [&](CPU* theThis) func
#define CPUHOOKPTR std::function<void(CPU* theThis)>
#define COPY(x) x=theSource->x

#ifndef RAPT_LEGACY //(COMMUNITY_2026)
#define RAPT_LEGACY //(COMMUNITY_2026)
#endif //(COMMUNITY_2026)

#ifndef LEGACY_GL //(COMMUNITY_2026)
#define LEGACY_GL //(COMMUNITY_2026)
#endif //(COMMUNITY_2026)

//
// The CPU class replaces the Gadget class from the old framework.
// A CPU is an object that is intended to act as a "program" in its 
// own right.  You can add CPUs to one to make them `child CPUs.
//
// CPUs respond to mouse/keyboard/system events.
//
```
---

## 7) Replace CMakeLists.txt
The CMakeLists.txt file included in the source code is old, inefficient and contains a few errors.

At the bottom of this file is the updated CMakeLists.txt file, replace the original with this version.

---

## 8) Update AppData variable
Change the name of the game in system directory from RWK to RWK_COM:

Edit C:\RWK_Source\RWK_Source\Games\RWK\Source\MyApp.cpp around line 95:

	//SetAppName("RWK");
	SetAppName("RWK_COM"); //(COMMUNITY_2026)
	SetAppVersion("Alpha 0");

---

## 9) Clean build folder (optional)
If you previously attempted builds, delete the CMake build output folder(s), e.g.:

```
C:\RWK_Source\RWK_Source\Games\RWK\Project\CMakeWin\cmake-build-*
```
Where * equals the ame of the profile, for example **cmake-build-Debug-VisualStudio-86**

Then:
- **Reload CMake Project**
- **Build RWK**

---

## 10) Build target RWK
Use the Build (hammer) icon and build the `RWK` target.

Expected output:

```
C:\RWK_Source\RWK_Source\Games\RWK\Resources\RWK.exe
```

---

## 11) Run the game
Run:

```
C:\RWK_Source\RWK_Source\Games\RWK\Resources\RWK.exe
```

If Windows reports missing DLLs, re-check Step 2 (DLL copy). No `bass.dll` is needed any more.

At times, the game will continue to run after you close it, and won't be able to recompile. If needed, force close 'RWK.exe' from Task Manager.

---

# Troubleshooting

## “Missing DLL” when launching RWK.exe
Make sure you copied **all** DLLs from:

```
C:\RWK_Source\RWK_Source\Framework\OS\Windows\DLLs\
```

into:

```
C:\RWK_Source\RWK_Source\Games\RWK\Resources\
```

(`bass.dll` is no longer used — BASS has been removed from the engine.)

## Linker errors about x86 vs x64
Your toolchain is still building x64. Re-check:

- Toolchains → **Architecture: x86**

## Linker error: multiply defined symbols (MLObject / MLText / etc.)
This happens if both RWK’s `MLRender.cpp` and the framework `rapt_ml.cpp` are built together.

Our working `CMakeLists.txt` (below) excludes `rapt_ml.cpp` automatically.

---

# Working CMakeLists.txt (Windows)

This is the known-good `CMakeLists.txt` that produces a working build (x86).

```cmake
cmake_minimum_required(VERSION 3.20)
project(RWK VERSION 0.1.0 LANGUAGES C CXX)

set(APPNAME "RWK")

# ---- Absolute roots based on THIS file location ----
# ...Games/RWK/Project/CMakeWin -> up 4 -> ...RWK_Source
get_filename_component(RWK_TOP "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)

set(RWK_GAME_DIR     "${RWK_TOP}/Games/RWK")
set(RWK_SOURCE_DIR   "${RWK_GAME_DIR}/Source")
set(RWK_RES_DIR      "${RWK_GAME_DIR}/Resources")

set(FW_ROOT          "${RWK_TOP}/Framework")
set(FW_RAPT_DIR      "${FW_ROOT}/RAPT")
set(FW_WIN_DIR       "${FW_ROOT}/OS/Windows")
set(FW_OS_DIR        "${FW_ROOT}/OS")

set(SDL_LIB_DIR      "${FW_WIN_DIR}/SDL_2020/Lib")
set(SDL_INC_DIR      "${FW_WIN_DIR}/SDL_2020/Include")

# ---- Sanity checks ----
if (NOT EXISTS "${FW_RAPT_DIR}/rapt.h")
    message(FATAL_ERROR "rapt.h NOT FOUND at: ${FW_RAPT_DIR}/rapt.h")
else()
    message(STATUS "rapt.h found: ${FW_RAPT_DIR}/rapt.h")
endif()

# ---- Sources ----
file(GLOB_RECURSE RWK_SRCS CONFIGURE_DEPENDS
        "${RWK_SOURCE_DIR}/*.cpp"
)

file(GLOB FW_WIN_SRCS CONFIGURE_DEPENDS
        "${FW_WIN_DIR}/*.cpp"
)

# ---- RAPT sources ----
file(GLOB FW_RAPT_SRCS CONFIGURE_DEPENDS
        "${FW_RAPT_DIR}/*.cpp"
)

# RWK provides its own ML implementation (MLRender.*).
# The framework also has rapt_ml.cpp which defines the same symbols.
# Compile ONLY ONE, so we exclude the framework ML file.
list(FILTER FW_RAPT_SRCS EXCLUDE REGEX ".*/rapt_ml\\.cpp$")

set(MAIN_SRC "${CMAKE_CURRENT_LIST_DIR}/Main_Win.cpp")

# Optional: exclude MagicAd.cpp if it exists anywhere in game sources
list(FILTER RWK_SRCS EXCLUDE REGEX ".*/MagicAd\\.cpp$")

# ---- Target ----
add_executable(${APPNAME} WIN32
        ${MAIN_SRC}
        ${RWK_SRCS}
        ${FW_WIN_SRCS}
        ${FW_RAPT_SRCS}
)

# Put the EXE directly into Resources (where DLLs/assets live)
set_target_properties(${APPNAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
        RUNTIME_OUTPUT_DIRECTORY "${RWK_RES_DIR}"
)

# ---- Includes (target-scoped) ----
target_include_directories(${APPNAME} PRIVATE
        "${FW_WIN_DIR}/Include"
        "${FW_WIN_DIR}"
        "${FW_ROOT}/OS"
        "${FW_RAPT_DIR}"
        "${SDL_INC_DIR}"
        "${RWK_SOURCE_DIR}"
)

# ---- Compile defs (target-scoped) ----
target_compile_definitions(${APPNAME} PRIVATE
        _WIN32
        AppRunsInThread
        LEGACY_GL
        GL_LEGACY
        RAPT_LEGACY
        $<$<CONFIG:Debug>:_DEBUG>
)

# ---- Link libs ----
target_link_libraries(${APPNAME} PRIVATE
        "${SDL_LIB_DIR}/OpenGL32.lib"
        "${SDL_LIB_DIR}/glew32.lib"
        "${SDL_LIB_DIR}/SDL2.lib"
        "${SDL_LIB_DIR}/SDL2_image.lib"
)

if (MSVC)
    target_compile_options(${APPNAME} PRIVATE /W3)
endif()
```

---

## Optional: “Why x86?”
The libraries shipped inside the repo (`SDL2.lib`, `SDL2_image.lib`, `glew32.lib`) are **32-bit** in the provided framework bundle. A 64-bit build will not link cleanly against 32-bit `.lib` files.

If we later want x64, the correct solution would be to replace those libs with **x64** equivalents and then switch the toolchain to x64.

---

## Next steps (project direction)
Once the community can build reliably, the next major milestones are:
- Make a new MakerMall endpoint + tooling for community levels
- Improve Windows/PC support (modern input, scaling, controller UX)
- Add new blocks/powerups/enemies in a structured way
- Long-term modernization (renderer, audio, platform builds)
