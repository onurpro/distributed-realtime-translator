cmake_minimum_required(VERSION 3.18)

# --- SYSTEM CONFIG ---
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
# ---------------------

project(Distributed_Translator VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static" FORCE)

# --- GLOBAL FLAGS ---
set(GGML_NATIVE OFF CACHE BOOL "Disable native march" FORCE)
set(GGML_CPU_ARM_ARCH "armv8-a" CACHE STRING "Force ARM Arch" FORCE)

# --- SELECTION SWITCH ---
if(NOT DEFINED BOARD)
    set(BOARD "1" CACHE STRING "Select Board to Build (1=STT, 2=Translator)")
endif()

if(BOARD STREQUAL "1")
    message(STATUS ">>> CONFIGURING FOR BOARD 1: WHISPER (STT)")
    set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    
    add_subdirectory(ext/whisper.cpp)

elseif(BOARD STREQUAL "2")
    message(STATUS ">>> CONFIGURING FOR BOARD 2: LLAMA (TRANSLATOR)")
    set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)
    
    add_subdirectory(ext/llama.cpp)
elseif(BOARD STREQUAL "3")
    message(STATUS ">>> CONFIGURING FOR BOARD 3: PIPER (TTS)")
endif()

# These must be OUTSIDE the IF so they always run
add_subdirectory(app)
add_subdirectory(hal)