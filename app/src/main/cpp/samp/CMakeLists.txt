cmake_minimum_required(VERSION 3.12)
project(samp)

set(CMAKE_CXX_STANDARD 20)

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    add_definitions(-DVER_x32=true)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    add_definitions(-DVER_x32=false)
endif()

message(WARNING "-------------------------------------------------")
message(WARNING "")
message(WARNING "Author: github.com/kuzia15")
message(WARNING "Current: libsamp.so [${ANDROID_ABI}]")
message(WARNING "")
message(WARNING "-------------------------------------------------")

# Add source files
file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/**/**/**/*.c*
        )

# Exclude test files
list(FILTER SOURCES EXCLUDE REGEX ".*/vendor/rapidjson/test/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/vendor/rapidjson/example/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/vendor/rapidjson/doc/.*")

# Exclude duplicate files from old structure (keeping modular structure)
list(FILTER SOURCES EXCLUDE REGEX ".*/game/actor\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/game/game\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/game/object\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/game/playerped\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/game/vehicle\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/game/patches\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/game/pad\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/game/Entity/Object\\.cpp$")

# Add include directories
include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/util/
        ${CMAKE_CURRENT_LIST_DIR}/vendor/armhook
        ${CMAKE_CURRENT_LIST_DIR}/vendor/imgui
        ${CMAKE_CURRENT_LIST_DIR}/vendor/
        ${CMAKE_CURRENT_LIST_DIR}/game/RW/
        ${CMAKE_CURRENT_LIST_DIR}/game/Core/
        ${CMAKE_CURRENT_LIST_DIR}/game_sa/
)

# Opus
#add_library(libopus STATIC IMPORTED)
#set_target_properties(libopus PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/vendor/opus/libopus.a)

add_library(samp SHARED ${SOURCES})
target_link_libraries(samp log EGL GLESv3 opus shadowhook::shadowhook ${CMAKE_CURRENT_SOURCE_DIR}/vendor/bass/libs/${ANDROID_ABI}/libbass.so)

# Set compilation flags
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -s -g -fvisibility=hidden -pthread -Wall -O1 -fexceptions")

