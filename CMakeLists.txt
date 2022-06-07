cmake_minimum_required(VERSION 3.18)
project(karafuto_core)

set(CMAKE_CXX_STANDARD 20)

set(MAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

# Grab all sources
file(GLOB_RECURSE CPP_HEADERS ${MAIN_SOURCE_DIR}/*.hpp ${MAIN_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE CPP_SOURCES ${MAIN_SOURCE_DIR}/*.cpp)

# Select platform and default backend if not set
if (NOT EXISTS PLATFORM OR NOT EXISTS BACKEND)
    if (ANDROID)
        set(PLATFORM "Android")
        set(BACKEND "Vulkan")
    elseif (APPLE)
        set(PLATFORM "APPLE")
        set(BACKEND "Vulkan")
    elseif (UNIX)
        set(PLATFORM "UNIX")
        set(BACKEND "Vulkan")
    elseif (WIN32)
        set(PLATFORM "Windows")
        set(BACKEND "Vulkan")
        # set(BACKEND "OpenCL")
    endif ()
endif ()


message("🖥️ Current platform: ${PLATFORM}")
message("🖼️ Current backend: ${BACKEND} \n")

# Exclude everything platform-dependent
if (ON)
    set(CPP_HEADERS_ADDITIONALS "")
    file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
            ${MAIN_SOURCE_DIR}/core/contexts/network/*.hpp
            ${MAIN_SOURCE_DIR}/core/contexts/rendering/*.hpp
            ${MAIN_SOURCE_DIR}/core/contexts/rendering/*.inl
            )

    set(CPP_SOURCES_ADDITIONALS "")
    file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
            ${MAIN_SOURCE_DIR}/core/contexts/network/*.cpp
            ${MAIN_SOURCE_DIR}/core/contexts/rendering/*.cpp
            )

    list(REMOVE_ITEM CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
    list(REMOVE_ITEM CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
endif ()

# Create temporary variables contain platform-dependent sources
if (ON)
    message("🖊️ Grab dependent sources")

    if (${PLATFORM} STREQUAL "Android")
        message("\t - Basic HTTP adapter")
        if (ON)
            set(CPP_HEADERS_ADDITIONALS "")
            file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/network/basic/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/debug/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/debug/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/network/INetworkContext.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/INetworkRequest.hpp)

            set(CPP_SOURCES_ADDITIONALS "")
            file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/network/basic/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/debug/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/INetworkContext.cpp)

            list(APPEND CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
            list(APPEND CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
        endif ()

        if (${BACKEND} STREQUAL "Vulkan")
            message("\t - Vulkan adapter")
            set(CPP_HEADERS_ADDITIONALS "")
            file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/vulkan/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/vulkan/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/one-to-one/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/ImageResult.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/IRenderContext.hpp)

            set(CPP_SOURCES_ADDITIONALS "")
            file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/vulkan/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/one-to-one/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/IRenderContext.cpp)

            list(APPEND CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
            list(APPEND CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
        endif ()
    endif ()

    if (${PLATFORM} STREQUAL "Windows")
        message("\t - Basic HTTP adapter")
        if (ON)
            set(CPP_HEADERS_ADDITIONALS "")
            file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/network/basic/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/debug/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/debug/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/network/INetworkContext.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/INetworkRequest.hpp)

            set(CPP_SOURCES_ADDITIONALS "")
            file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/network/basic/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/debug/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/network/INetworkContext.cpp)

            list(APPEND CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
            list(APPEND CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
        endif ()

        if (${BACKEND} STREQUAL "Vulkan")
            message("\t - Vulkan adapter")
            set(CPP_HEADERS_ADDITIONALS "")
            file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/vulkan/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/vulkan/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/one-to-one/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/ImageResult.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/IRenderContext.hpp)

            set(CPP_SOURCES_ADDITIONALS "")
            file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/vulkan/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/one-to-one/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/IRenderContext.cpp)

            list(APPEND CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
            list(APPEND CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
        elseif (${BACKEND} STREQUAL "OpenCL")
            message("\t - OpenCL adapter")
            set(CPP_HEADERS_ADDITIONALS "")
            file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/opencl/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/opencl/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.inl
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/one-to-one/*.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/ImageResult.hpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/IRenderContext.hpp)

            set(CPP_SOURCES_ADDITIONALS "")
            file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/opencl/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/debug/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/one-to-one/*.cpp
                    ${MAIN_SOURCE_DIR}/core/contexts/rendering/IRenderContext.cpp)

            list(APPEND CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
            list(APPEND CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
        endif ()
    endif ()
endif ()

# Declare specific platform definitions
if (ON)
    message("📰️ Make platform-specific declarations")

    if (${PLATFORM} STREQUAL "Windows")
        # find better way to do it
        set(OCL_ROOT "C:/Program Files (x86)/OCL_SDK_Light")

        message("\t - Declare specific C-math definitions")
        add_compile_definitions(_USE_MATH_DEFINES)
    endif ()

    if (OFF)
        #        if (APPLE)
        #            message("MacOS: Override OpenSSL dirs")
        #
        #            # This is a bug in CMake that causes it to prefer the system version over
        #            # the one in the specified ROOT folder.
        #            set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR} /usr/local/Cellar/openssl@3/3.0.1/)
        #            set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
        #            set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
        #        endif ()
    endif ()
endif ()

# Declare libraries
if (ON)
    message("🧾 Declare libraries")

    if (OFF)
        #if (APPLE)
        #    message("MacOS: Added curl as shared library")
        #
        #    set(BUILD_SHARED_LIBS ON)
        #    set(HTTP_ONLY ON)
        #    add_subdirectory(${THIRD_PARTY_DIR}/curl)
        #    set(BUILD_SHARED_LIBS ON)
        #
        #    message("MacOS: Added opencl as static library")
        #    find_package(OpenCL REQUIRED)
        #
        #    message("MacOS: Added glm headers")
        #    set(BUILD_SHARED_LIBS OFF)
        #    add_subdirectory(${THIRD_PARTY_DIR}/glm EXCLUDE_FROM_ALL)
        #    set(BUILD_SHARED_LIBS OFF)
        #endif ()
        #
        #if (UNIX AND NOT APPLE)
        #    message("Unix: Added curl as shared library")
        #
        #    set(BUILD_SHARED_LIBS ON)
        #    set(HTTP_ONLY ON)
        #    add_subdirectory(${THIRD_PARTY_DIR}/curl)
        #    set(BUILD_SHARED_LIBS ON)
        #
        #    message("Unix: Added opencl as static library")
        #    find_package(OpenCL REQUIRED)
        #
        #    message("Unix: Added glm headers")
        #    set(BUILD_SHARED_LIBS OFF)
        #    add_subdirectory(${THIRD_PARTY_DIR}/glm EXCLUDE_FROM_ALL)
        #    set(BUILD_SHARED_LIBS OFF)
        #endif ()
    endif ()

    if (${PLATFORM} STREQUAL "Android")
        message("\t - Added HTTPRequest")
        add_subdirectory(${THIRD_PARTY_DIR}/HTTPRequest)

        message("\t - Added GLM...")
        add_subdirectory(${THIRD_PARTY_DIR}/glm)

        message("\t - Added LRUCache17...")
        add_subdirectory(${THIRD_PARTY_DIR}/lrucache17)
    endif ()

    if (${PLATFORM} STREQUAL "Windows")
        message("\t - Added HTTPRequest")
        add_subdirectory(${THIRD_PARTY_DIR}/HTTPRequest)

        message("\t - Added GLM...")
        add_subdirectory(${THIRD_PARTY_DIR}/glm)

        message("\t - Added LRUCache17...")
        add_subdirectory(${THIRD_PARTY_DIR}/lrucache17)

        #        message("\t - Added zlib...")
        #        if (ON)
        #            add_subdirectory(${THIRD_PARTY_DIR}/zlib)
        #
        #            message(${CMAKE_BINARY_DIR}/third-party/zlib)
        #
        #            set(ZLIB_INCLUDE_DIR ${THIRD_PARTY_DIR}/zlib "${CMAKE_BINARY_DIR}/third-party/zlib")
        #            set(ZLIB_LIBRARY zlib)
        #        endif ()
    endif ()
endif ()

# Declare targets
if (ON)
    message("")
    message("🎯 Declare targets")

    if (${PLATFORM} STREQUAL "Android")
        add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
    elseif (${PLATFORM} STREQUAL "Windows")
        add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
        add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})
    endif()

    if (OFF)
        #if (APPLE)
        #    add_library(libkcore-static STATIC ${CPP_HEADERS} ${CPP_SOURCES})
        #    add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
        #    add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})
        #
        #    set(LIBRARIES libcurl glm "-framework OpenCL")
        #
        #    target_link_libraries(libkcore PUBLIC ${LIBRARIES})
        #    target_link_libraries(libkcore-static PUBLIC ${LIBRARIES})
        #    target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
        #
        #    target_link_directories(libkcore PRIVATE ${OpenCL_LIBRARY})
        #    target_link_directories(libkcore-static PRIVATE ${OpenCL_LIBRARY})
        #    target_link_directories(karafuto_core PRIVATE ${OpenCL_LIBRARY})
        #endif ()
        #
        #if (UNIX AND NOT APPLE)
        #    add_library(libkcore-static STATIC ${CPP_HEADERS} ${CPP_SOURCES})
        #    add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
        #    add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})
        #
        #    set(LIBRARIES libcurl glm OpenCL pthread)
        #
        #    target_link_libraries(libkcore PUBLIC ${LIBRARIES})
        #    target_link_libraries(libkcore-static PUBLIC ${LIBRARIES})
        #    target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
        #
        #    target_link_directories(libkcore PRIVATE ${OpenCL_LIBRARY})
        #    target_link_directories(libkcore-static PRIVATE ${OpenCL_LIBRARY})
        #    target_link_directories(karafuto_core PRIVATE ${OpenCL_LIBRARY})
        #endif ()
    endif ()

    message("\t - Link libraries")
    if (ON)
        if (${PLATFORM} STREQUAL "Android")
            if (${BACKEND} STREQUAL "Vulkan")
                message("\t - Vulkan libraries")
                set(LIBRARIES glm HTTPRequest vulkan)
                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
            endif()
        elseif (${PLATFORM} STREQUAL "Windows")
            if (${BACKEND} STREQUAL "Vulkan")
                message("\t - Vulkan libraries")
                set(LIBRARIES glm HTTPRequest wsock32 ws2_32)

                find_package(Vulkan REQUIRED)

                target_link_libraries(libkcore PUBLIC ${LIBRARIES} ${Vulkan_LIBRARIES})
                # target_link_libraries(libkcore-static PUBLIC ${LIBRARIES} ${Vulkan_LIBRARIES})
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES} ${Vulkan_LIBRARIES})
            elseif (${BACKEND} STREQUAL "OpenCL")
                message("\t - OpenCL libraries")
                set(LIBRARIES opencl glm HTTPRequest wsock32 ws2_32)

                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
                target_link_directories(libkcore PUBLIC ${LIBRARIES} ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)
                # target_link_directories(libkcore-static PRIVATE ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
                target_link_directories(karafuto_core PUBLIC ${LIBRARIES} ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)
            endif ()
        endif ()
    endif ()

    message("\t - Add headers")
    if (ON)
        if (OFF)
            #            if (APPLE)
            #                message("MacOS: Added RapidJSON, Earcut, stb-image includes")
            #                set(INCLUDE_COMPOUND
            #                        ${THIRD_PARTY_DIR}/rapidjson/include
            #                        ${THIRD_PARTY_DIR}/earcut/include
            #                        ${THIRD_PARTY_DIR}/stb-image
            #                        ${THIRD_PARTY_DIR}/gzip-hpp/include
            #                        )
            #                target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
            #                target_include_directories(libkcore-static PUBLIC ${INCLUDE_COMPOUND})
            #                target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
            #            endif ()
            #
            #            if (UNIX AND NOT APPLE)
            #                message("Unix: Added RapidJSON, Earcut, stb-image, gzip includes")
            #                set(INCLUDE_COMPOUND
            #                        ${THIRD_PARTY_DIR}/rapidjson/include
            #                        ${THIRD_PARTY_DIR}/earcut/include
            #                        ${THIRD_PARTY_DIR}/stb-image
            #                        ${THIRD_PARTY_DIR}/gzip-hpp/include
            #                        )
            #                target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
            #                target_include_directories(libkcore-static PUBLIC ${INCLUDE_COMPOUND})
            #                target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
            #            endif ()
        endif ()

        if (${PLATFORM} STREQUAL "Android")
            set(INCLUDE_COMPOUND
                    ${THIRD_PARTY_DIR}/rapidjson/include
                    ${THIRD_PARTY_DIR}/lrucache17
                    ${THIRD_PARTY_DIR}/earcut/include
                    ${THIRD_PARTY_DIR}/stb-image
                    )
            target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
        elseif (${PLATFORM} STREQUAL "Windows")
            set(INCLUDE_COMPOUND
                    ${THIRD_PARTY_DIR}/rapidjson/include
                    ${THIRD_PARTY_DIR}/lrucache17
                    ${THIRD_PARTY_DIR}/earcut/include
                    ${THIRD_PARTY_DIR}/stb-image
                    ${OCL_ROOT}/include

                    ${THIRD_PARTY_DIR}/zlib
                    ${CMAKE_BINARY_DIR}/third-party/zlib
                    $ENV{VULKAN_SDK}/Include
                    )
            target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
            include_directories(${THIRD_PARTY_DIR}/zlib)
            # target_include_directories(libkcore-static PUBLIC ${INCLUDE_COMPOUND})
            target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
        endif ()
    endif ()
endif ()
