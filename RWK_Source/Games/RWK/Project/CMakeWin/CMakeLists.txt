# CMAKE FILE FOR A WINDOWS BUILD
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
# BASS is gone: Sound_Core is SDL2 + the vendored public-domain stb_vorbis
# (Framework/OS/common_sound_core.cpp), and SDL2.lib is already linked below.

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