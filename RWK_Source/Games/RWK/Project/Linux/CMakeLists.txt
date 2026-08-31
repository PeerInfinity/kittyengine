set(appname "kittyengine")
cmake_minimum_required(VERSION 3.20.0)
project(${appname} VERSION 0.1.0 LANGUAGES C CXX)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../Resources/)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_COMPILER "/usr/bin/clang")
set( CMAKE_CXX_STANDARD 17 )

# Getting all c++ source files

file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/../../Source/*.cpp)
file(GLOB _OS ${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/*.cpp)
file(GLOB _RAPT ${PROJECT_SOURCE_DIR}/../../../../Framework/RAPT/*.cpp)
file(GLOB_RECURSE MAIN ${PROJECT_SOURCE_DIR}/main.cpp)

set(RAPT_SDL2_LOCAL OFF)
set(RAPT_GLEW_LOCAL OFF)
set(RAPT_CURL_LOCAL OFF)

# Option to switch between a system sdl library and a sdl library from in project
# source code
option(RAPT_SDL2_LOCAL "Use non-system SDL2 library" OFF)
# Same thing but for GLEW
option(RAPT_GLEW_LOCAL "Use non-system GLEW library" OFF)

# Copied almost verbatim from here https://wiki.libsdl.org/SDL2/README/cmake#including-sdl-in-your-project
# This either gets SDL2 from it's CMakeLists.txt in source or links it from the system dll
if(RAPT_SDL2_LOCAL)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/ThirdParty/SDL2 EXCLUDE_FROM_ALL ${CMAKE_CURRENT_BINARY_DIR}/SDL2)
else()
    # 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
    # find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

    find_package(SDL2_Image REQUIRED)
endif()

# Get OpenGL since we need it
find_package(OpenGL REQUIRED)

# find glew either by local source code for just link to system dll
if(RAPT_GLEW_LOCAL)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/ThirdParty/GLEW/build/cmake EXCLUDE_FROM_ALL ${CMAKE_CURRENT_BINARY_DIR}/GLEW)
else()
    find_package(GLEW REQUIRED)
endif()

# Including framework CMakeLists.txt or sources?
# no seriously dad, what is this?
include_directories(${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/ThirdParty/)
include_directories(${PROJECT_SOURCE_DIR}/../../../../Framework/RAPT/)
include_directories(${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux)
include_directories(${PROJECT_SOURCE_DIR}/../../../../Framework/OS/)
include_directories(${PROJECT_SOURCE_DIR}/../../Source/)
# include_directories(${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/ThirdParty/CURL/curl)


# Main executable
add_executable(${appname} ${SRCS} ${MAIN} ${_OS} ${_RAPT})

target_include_directories(${appname} PRIVATE /usr/include)
# Show all warnings, works for clang and GCC
#target_compile_options(${appname} PRIVATE -Wall -Wno-extra -Wno-comment -Wno-write-strings -Wno-class-memaccess -Wno-missing-field-initializers -Wno-unused-variable -Wno-deprecated-copy -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-tautological-compare -Wno-conversion-null -Wno-misleading-indentation -fmax-errors=1 -Wfatal-errors -Wno-delete-incomplete -Wno-int-to-pointer-cast -DGL30 -Wno-format-security -fpermissive) #gcc

#clang
target_compile_options(${appname} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-pthread -Wall -Wno-extra -Wno-comment -Wno-write-strings -Wno-missing-field-initializers -Wno-unused-variable -Wno-deprecated-copy -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-tautological-compare -Wno-conversion-null -Wno-misleading-indentation -Wfatal-errors -Wno-delete-incomplete -Wno-int-to-pointer-cast -Wno-format-security -fpermissive -Wno-bitwise-instead-of-logical -Wno-overloaded-virtual -Wno-dynamic-class-memaccess -Wno-delete-non-abstract-non-virtual-dtor -Wno-char-subscripts -Wno-constant-conversion -Wno-unused-but-set-variable -Wno-unsequenced -Wno-unknown-warning-option>)
target_compile_options(${appname} PRIVATE $<$<COMPILE_LANGUAGE:C>:-pthread -Wall -Wno-unused-variable>) #clang

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif ()
add_definitions(-DLEGACY_GL -DGL_LEGACY -DRAPT_LEGACY -DNO_RAPT_ML) # for legacy system builds only
#add_definitions(-DGL30) # for non-legacy system builds only

target_compile_definitions(${appname} PRIVATE _LINUX AppRunsInThread)

# Set c++ standard for executable
set_target_properties(${appname} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS ON INSTALL_RPATH "$ORIGIN" BUILD_WITH_INSTALL_RPATH TRUE)

# Link to the actual SDL2 library.
# SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL library.

if(NOT RAPT_SDL2_LOCAL)
    target_link_libraries(${appname} PRIVATE SDL2::SDL2)
    target_link_libraries(${appname} PRIVATE SDL2_image::SDL2_image)    
else()
    target_link_libraries(${appname} PRIVATE SDL2::SDL2-static)
endif()

if (NOT RAPT_CURL_LOCAL)
    find_package(CURL REQUIRED)
    include_directories(${CURL_INCLUDE_DIR})
    target_link_libraries(${appname} PRIVATE ${CURL_LIBRARIES})
else()
    set(LIBCURL_PATH "${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/ThirdParty/CURL/")
    target_link_libraries(${appname} PRIVATE ${LIBCURL_PATH}libcurl.a ${LIBCURL_PATH}libssl.a ${LIBCURL_PATH}libcrypto.a)
endif()

# Audio needs no third-party library any more: Sound_Core is SDL2 + the
# vendored public-domain stb_vorbis (Framework/OS/common_sound_core.cpp), and
# SDL2 is already linked above.  BASS is gone -- with it the libbass.so
# runtime dependency and the LD_LIBRARY_PATH that used to be needed to run.

#
# This strips the /framework/os/blah info from the final executable...
#
set(CMAKE_BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
#
#

# Link to OpenGL
#target_link_libraries(${appname} PRIVATE OpenGL::OpenGL ${CURL_LIBRARIES} ${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/ThirdParty/SDLImage/libSDL2_image.a )
# target_link_libraries(${appname} PRIVATE OpenGL::OpenGL ${PROJECT_SOURCE_DIR}/../../../../Framework/OS/Linux/ThirdParty/SDLImage/libSDL2_image.a)
target_link_libraries(${appname} PRIVATE OpenGL::OpenGL )

# Link to GLEW
if(NOT RAPT_GLEW_LOCAL)
    target_link_libraries(${appname} PRIVATE ${GLEW_LIBRARIES})
else()
    # Static version of GLEW
    target_link_libraries(${appname} PRIVATE glew_s)
endif()
