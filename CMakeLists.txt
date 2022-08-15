cmake_minimum_required(VERSION 3.4)
project(karafuto_core)

set(CMAKE_CXX_STANDARD 20)

set(MAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Grab all sources
file(GLOB_RECURSE CPP_HEADERS ${MAIN_SOURCE_DIR}/*.hpp ${MAIN_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE CPP_SOURCES ${MAIN_SOURCE_DIR}/*.cpp)

# Select platform and default backend if not set
if (NOT EXISTS PLATFORM)
    if (ANDROID)
        set(PLATFORM "Android")
    elseif (APPLE)
        set(PLATFORM "MacOS")
    elseif (UNIX)
        set(PLATFORM "Linux")
    elseif (WIN32)
        set(PLATFORM "Windows")
    elseif ()
        message(FATAL_ERROR "Undefined target platform")
    endif ()
endif ()

message("🖥️ Current platform: ${PLATFORM}")

# Declare specific platform definitions
if (${PLATFORM} STREQUAL "Windows")
    message("📰️ Make platform-specific declarations")

    if (MSVC)
        message("\t - Declare specific C-math definitions")
        add_compile_definitions(_USE_MATH_DEFINES)

        # build msvc
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
    endif ()
endif ()

# Declare libraries
if (ON)
    message("🧾 Declare libraries")

    message("\t - Added HTTPRequest")
    add_subdirectory(${THIRD_PARTY_DIR}/HTTPRequest)

    message("\t - Added GLM...")
    add_subdirectory(${THIRD_PARTY_DIR}/glm)

    message("\t - Added LRUCache17...")
    add_subdirectory(${THIRD_PARTY_DIR}/lrucache17)
endif ()

# Declare targets
if (ON)
    message("")
    message("🎯 Declare targets")

    add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
    add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})

    message("\t - Link libraries")
    if (${PLATFORM} STREQUAL "Windows")
        set(LIBRARIES glm HTTPRequest wsock32 ws2_32)
        target_link_libraries(libkcore PRIVATE ${LIBRARIES})
        target_link_libraries(karafuto_core PRIVATE ${LIBRARIES})
    else ()
        set(LIBRARIES glm HTTPRequest)
        target_link_libraries(libkcore PRIVATE ${LIBRARIES})
        target_link_libraries(karafuto_core PRIVATE ${LIBRARIES})
    endif ()

    message("\t - Add headers")
    if (${PLATFORM} STREQUAL "Windows")
        set(INCLUDE_COMPOUND
                ${THIRD_PARTY_DIR}/rapidjson/include
                ${THIRD_PARTY_DIR}/lrucache17
                ${THIRD_PARTY_DIR}/earcut/include
                ${THIRD_PARTY_DIR}/stb-image)
        target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
        target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
    else ()
        set(INCLUDE_COMPOUND
                ${THIRD_PARTY_DIR}/rapidjson/include
                ${THIRD_PARTY_DIR}/lrucache17
                ${THIRD_PARTY_DIR}/earcut/include
                ${THIRD_PARTY_DIR}/stb-image)
        target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
        target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
    endif ()
endif ()
