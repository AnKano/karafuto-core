cmake_minimum_required(VERSION 3.21)
project(karafuto_core)

set(CMAKE_CXX_STANDARD 17)

set(MAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)


# ----|> declare sources

file(GLOB_RECURSE CPP_HEADERS ${MAIN_SOURCE_DIR}/*.hpp ${MAIN_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE CPP_SOURCES ${MAIN_SOURCE_DIR}/*.cpp)

set(CPP_HEADERS_EXCLUDE "")
set(CPP_SOURCES_EXCLUDE "")

if (EMSCRIPTEN)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ems-server/static)
    set(CMAKE_EXECUTABLE_SUFFIX .js)

    file(GLOB_RECURSE CPP_HEADERS_EXCLUDE
            ${MAIN_SOURCE_DIR}/core/contexts/network/curl/*.hpp

            ${MAIN_SOURCE_DIR}/core/contexts/rendering/opencl/*.hpp
            ${MAIN_SOURCE_DIR}/core/contexts/rendering/opencl/*.inl

            ${MAIN_SOURCE_DIR}/core/contexts/rendering/opengl/*.hpp
            ${MAIN_SOURCE_DIR}/core/contexts/rendering/opengl/*.inl
            )
    file(GLOB_RECURSE CPP_SOURCES_EXCLUDE
            ${MAIN_SOURCE_DIR}/core/contexts/network/curl/*.cpp

            ${MAIN_SOURCE_DIR}/core/contexts/rendering/opencl/*.cpp

            ${MAIN_SOURCE_DIR}/core/contexts/rendering/opengl/*.cpp
            )

    list(REMOVE_ITEM CPP_HEADERS ${CPP_HEADERS_EXCLUDE})
    list(REMOVE_ITEM CPP_SOURCES ${CPP_SOURCES_EXCLUDE})

    add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})

    add_subdirectory(${THIRD_PARTY_DIR}/glm EXCLUDE_FROM_ALL)

    set(INCLUDE_COMPOUND
            ${THIRD_PARTY_DIR}/gzip-hpp/include
            ${THIRD_PARTY_DIR}/rapidjson/include
            ${THIRD_PARTY_DIR}/earcut/include
            ${THIRD_PARTY_DIR}/stb-image)

    target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})

    target_link_libraries(karafuto_core glm)

    set(CMAKE_CXX_FLAGS "-pthread")
    add_link_options(-pthread)
    target_link_options(karafuto_core PUBLIC "SHELL: -s USE_ZLIB=1 -s EXPORTED_RUNTIME_METHODS=[\"setValue\",\"getValue\"] -lembind -s FETCH=1 -s MIN_WEBGL_VERSION=2 -s OFFSCREEN_FRAMEBUFFER=1 -s WASM=1 -s OFFSCREENCANVAS_SUPPORT=1 -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4 --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/build/assets@/assets")
else ()
    # select correct sources for current platform
    if (APPLE OR UNIX OR WIN32)
        # MacOS, Unix and Windows should use OpenCL by default

        if (APPLE)
            message("MacOS: Select OpenCL Rendering backend")
        endif ()

        if (UNIX)
            message("Unix: Select OpenCL Rendering backend")
        endif ()

        if (WIN32)
            message("Windows: Select OpenCL Rendering backend")
        endif ()

        #exclude OpenGL ES directories
        file(GLOB_RECURSE CPP_HEADERS_EXCLUDE
                ${MAIN_SOURCE_DIR}/core/contexts/rendering/opengl/*.hpp
                ${MAIN_SOURCE_DIR}/core/contexts/rendering/opengl/*.inl)
        file(GLOB_RECURSE CPP_SOURCES_EXCLUDE
                ${MAIN_SOURCE_DIR}/core/contexts/rendering/opengl/*.cpp)
    endif ()

    # remove unrelated sources
    list(REMOVE_ITEM CPP_HEADERS ${CPP_HEADERS_EXCLUDE})
    list(REMOVE_ITEM CPP_SOURCES ${CPP_SOURCES_EXCLUDE})


    # ----|> declare specific platform definitions
    if (WIN32)
        # find better way to do it
        set(OCL_ROOT "C:/Program Files (x86)/OCL_SDK_Light")

        message("Windows: Declare important math definitions")
        add_compile_definitions(_USE_MATH_DEFINES)
    endif ()

    if (APPLE)
        message("MacOS: Override OpenSSL dirs")

        # This is a bug in CMake that causes it to prefer the system version over
        # the one in the specified ROOT folder.
        set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR} /usr/local/Cellar/openssl@3/3.0.1/)
        set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
        set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
    endif ()


    # ----|> declare libraries

    if (APPLE)
        message("MacOS: Added curl as shared library")

        set(BUILD_SHARED_LIBS ON)
        set(HTTP_ONLY ON)
        add_subdirectory(${THIRD_PARTY_DIR}/curl)
        set(BUILD_SHARED_LIBS ON)

        message("MacOS: Added opencl as static library")
        find_package(OpenCL REQUIRED)

        message("MacOS: Added glm headers")
        set(BUILD_SHARED_LIBS OFF)
        add_subdirectory(${THIRD_PARTY_DIR}/glm EXCLUDE_FROM_ALL)
        set(BUILD_SHARED_LIBS OFF)
    endif ()

    if (UNIX AND NOT APPLE)
        message("Unix: Added curl as shared library")

        set(BUILD_SHARED_LIBS ON)
        set(HTTP_ONLY ON)
        add_subdirectory(${THIRD_PARTY_DIR}/curl)
        set(BUILD_SHARED_LIBS ON)

        message("Unix: Added opencl as static library")
        find_package(OpenCL REQUIRED)

        message("Unix: Added glm headers")
        set(BUILD_SHARED_LIBS OFF)
        add_subdirectory(${THIRD_PARTY_DIR}/glm EXCLUDE_FROM_ALL)
        set(BUILD_SHARED_LIBS OFF)
    endif ()

    if (WIN32)
        message("Windows: Added curl as shared library")

        set(BUILD_SHARED_LIBS ON)
        set(HTTP_ONLY ON)
        add_subdirectory(${THIRD_PARTY_DIR}/curl)
        set(BUILD_SHARED_LIBS ON)


        message("Windows: Added glm headers")
        set(BUILD_SHARED_LIBS OFF)
        add_subdirectory(${THIRD_PARTY_DIR}/glm EXCLUDE_FROM_ALL)
        set(BUILD_SHARED_LIBS OFF)

        message("Windows: Added zlib headers")
        add_subdirectory(${THIRD_PARTY_DIR}/zlib)

        message(${CMAKE_BINARY_DIR}/third-party/zlib)

        set(ZLIB_INCLUDE_DIR ${THIRD_PARTY_DIR}/zlib "${CMAKE_BINARY_DIR}/third-party/zlib")
        set(ZLIB_LIBRARY zlib)
    endif ()

    # ----|> declare targets

    if (APPLE)
        add_library(libkcore-static STATIC ${CPP_HEADERS} ${CPP_SOURCES})
        add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
        add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})

        set(LIBRARIES libcurl glm "-framework OpenCL")

        target_link_libraries(libkcore PUBLIC ${LIBRARIES})
        target_link_libraries(libkcore-static PUBLIC ${LIBRARIES})
        target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})

        target_link_directories(libkcore PRIVATE ${OpenCL_LIBRARY})
        target_link_directories(libkcore-static PRIVATE ${OpenCL_LIBRARY})
        target_link_directories(karafuto_core PRIVATE ${OpenCL_LIBRARY})
    endif ()

    if (UNIX AND NOT APPLE)
        add_library(libkcore-static STATIC ${CPP_HEADERS} ${CPP_SOURCES})
        add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
        add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})

        set(LIBRARIES libcurl glm OpenCL pthread)

        target_link_libraries(libkcore PUBLIC ${LIBRARIES})
        target_link_libraries(libkcore-static PUBLIC ${LIBRARIES})
        target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})

        target_link_directories(libkcore PRIVATE ${OpenCL_LIBRARY})
        target_link_directories(libkcore-static PRIVATE ${OpenCL_LIBRARY})
        target_link_directories(karafuto_core PRIVATE ${OpenCL_LIBRARY})
    endif ()

    if (WIN32)
        add_library(libkcore-static STATIC ${CPP_HEADERS} ${CPP_SOURCES})
        add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
        add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})

        set(LIBRARIES libcurl glm opencl zlibstatic)

        target_link_directories(libkcore PRIVATE ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)
        target_link_directories(libkcore-static PRIVATE ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)
        target_link_directories(karafuto_core PRIVATE ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)

        target_link_libraries(libkcore PUBLIC ${LIBRARIES})
        target_link_libraries(libkcore-static PUBLIC ${LIBRARIES})
        target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
    endif ()

    # ----|> declare targets

    if (APPLE)
        message("MacOS: Added RapidJSON, Earcut, stb-image includes")
        set(INCLUDE_COMPOUND
                ${THIRD_PARTY_DIR}/rapidjson/include
                ${THIRD_PARTY_DIR}/earcut/include
                ${THIRD_PARTY_DIR}/stb-image
                ${THIRD_PARTY_DIR}/gzip-hpp/include
                )
        target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
        target_include_directories(libkcore-static PUBLIC ${INCLUDE_COMPOUND})
        target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
    endif ()

    if (UNIX AND NOT APPLE)
        message("Unix: Added RapidJSON, Earcut, stb-image, gzip includes")
        set(INCLUDE_COMPOUND
                ${THIRD_PARTY_DIR}/rapidjson/include
                ${THIRD_PARTY_DIR}/earcut/include
                ${THIRD_PARTY_DIR}/stb-image
                ${THIRD_PARTY_DIR}/gzip-hpp/include
                )
        target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
        target_include_directories(libkcore-static PUBLIC ${INCLUDE_COMPOUND})
        target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
    endif ()

    if (WIN32)
        message("Windows: Added RapidJSON, Earcut, stb-image, gzip and opencl includes")
        set(INCLUDE_COMPOUND
                ${THIRD_PARTY_DIR}/rapidjson/include
                ${THIRD_PARTY_DIR}/earcut/include
                ${THIRD_PARTY_DIR}/stb-image
                ${THIRD_PARTY_DIR}/gzip-hpp/include
                ${OCL_ROOT}/include

                ${THIRD_PARTY_DIR}/zlib
                ${CMAKE_BINARY_DIR}/third-party/zlib
                )
        target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
        include_directories(${THIRD_PARTY_DIR}/zlib)
        target_include_directories(libkcore-static PUBLIC ${INCLUDE_COMPOUND})
        target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
    endif ()
endif ()