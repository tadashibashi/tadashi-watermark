# TADASHISOUND WATERMARK UTILITY
# Command line utility to add watermarks into mp3 files

cmake_minimum_required(VERSION 4.0)
project(tadashi_watermark)

set(CMAKE_CXX_STANDARD 20)

# ----- Dependencies ----------------------------------------------------------
# Find LAME (It must be installed locally)
find_library(LAME_LIBRARY
    NAMES mp3lame lame
    HINTS /usr/local/lib /opt/homebrew/opt/lame/lib
    REQUIRED
)
find_path(LAME_INCLUDE_DIR
    NAMES lame/lame.h
    HINTS /usr/local/include /opt/homebrew/opt/lame/include)

include(FetchContent)
FetchContent_Declare(miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio
    GIT_TAG        0.11.23
)
    set(MINIAUDIO_NO_ENCODING    ON CACHE BOOL "" FORCE)
    set(MINIAUDIO_NO_DEVICEIO    ON CACHE BOOL "" FORCE)
    set(MINIAUDIO_NO_RESOURCE_MANAGER    ON CACHE BOOL "" FORCE)
    set(MINIAUDIO_NO_NODE_GRAPH  ON CACHE BOOL "" FORCE)
    set(MINIAUDIO_NO_ENGINE      ON CACHE BOOL "" FORCE)
    set(MINIAUDIO_NO_THREADING   ON CACHE BOOL "" FORCE)
    set(MINIAUDIO_NO_GENERATION  ON CACHE BOOL "" FORCE)
    set(MINIAUDIO_NO_EXTRA_NODES ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(miniaudio)

FetchContent_Declare(ctpl
    GIT_REPOSITORY https://github.com/vit-vit/CTPL
    GIT_TAG        ctpl_v.0.0.2
)
FetchContent_MakeAvailable(ctpl)

add_library(ctpl INTERFACE)
target_include_directories(ctpl INTERFACE ${CMAKE_BINARY_DIR}/_deps/ctpl-src)

# ----- Declare Watermark Tool ------------------------------------------------
add_executable(tadashi_watermark
    src/main.cpp
    src/options.cpp
)
target_include_directories(tadashi_watermark PRIVATE
    ${LAME_INCLUDE_DIR}
    ${DR_LIBS_INCLUDE_DIR}
)
target_link_libraries(tadashi_watermark PRIVATE
    "${LAME_LIBRARY}"
    miniaudio
    ctpl
)
