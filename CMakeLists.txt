cmake_minimum_required(VERSION 3.21)
project(karafuto_core)

set(CMAKE_CXX_STANDARD 17)

set(MAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

file(GLOB_RECURSE CPP_HEADERS
        ${MAIN_SOURCE_DIR}/*.hpp
        ${MAIN_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE CPP_SOURCES
        ${MAIN_SOURCE_DIR}/*.cpp)

if (NOT EMSCRIPTEN)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

    if (WIN32)
        message("--Detected Windows!")
        add_compile_definitions(_USE_MATH_DEFINES)
    else ()
        message("--Detected UNIX!")
    endif ()

    # include described down libs as shared (.dll) dependencies
    set(BUILD_SHARED_LIBS ON)
    set(HTTP_ONLY ON)
    add_subdirectory(${THIRD_PARTY_DIR}/curl)

    add_subdirectory(${THIRD_PARTY_DIR}/glfw EXCLUDE_FROM_ALL)
    # rename libname for unifying
    set_target_properties(glfw PROPERTIES OUTPUT_NAME "libglfw3")
    set(BUILD_SHARED_LIBS ON)

    # include described down libs as static (built-in general .dll) dependencies
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(${THIRD_PARTY_DIR}/glew EXCLUDE_FROM_ALL)

    add_subdirectory(${THIRD_PARTY_DIR}/glm EXCLUDE_FROM_ALL)
    set(BUILD_SHARED_LIBS OFF)

    add_library(libkcore STATIC ${CPP_HEADERS} ${CPP_SOURCES})
    target_link_libraries(libkcore-static PUBLIC
            glfw
            libglew_static
            libcurl
            glm)

    add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
    target_link_libraries(libkcore PUBLIC
            glfw
            libglew_static
            libcurl
            glm)

    add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})
    target_link_libraries(karafuto_core PUBLIC
            glfw
            libglew_static
            libcurl
            glm)

    target_include_directories(libkcore PUBLIC ${THIRD_PARTY_DIR}/rapidjson/include)
    target_include_directories(libkcore-static PUBLIC ${THIRD_PARTY_DIR}/rapidjson/include)
    target_include_directories(karafuto_core PUBLIC ${THIRD_PARTY_DIR}/rapidjson/include)

    target_include_directories(libkcore PUBLIC ${THIRD_PARTY_DIR}/earcut/include)
    target_include_directories(libkcore-static PUBLIC ${THIRD_PARTY_DIR}/earcut/include)
    target_include_directories(karafuto_core PUBLIC ${THIRD_PARTY_DIR}/earcut/include)

    target_include_directories(libkcore PUBLIC ${THIRD_PARTY_DIR}/stb-image)
    target_include_directories(libkcore-static PUBLIC ${THIRD_PARTY_DIR}/stb-image)
    target_include_directories(karafuto_core PUBLIC ${THIRD_PARTY_DIR}/stb-image)
endif ()

#if (EMSCRIPTEN)
#    message("--Detected Emscripten!")
#
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
#    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
#    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
#
#    # set emscripten to .js environment creation (instead of .html or .wasm)
#    set(CMAKE_EXECUTABLE_SUFFIX ".html")
#
#    add_subdirectory(${THIRD_PARTY_DIR}/glm)
#
#    add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})
#
#    # build emscripten with options:
#    # 1. include all Emscripten binds functionality
#    # 2. add some optimizations
#    # 3. build for 'web' environment
#    # 4. add possibility to remap WebAssembly.Memory
#    # 5. build as module.export
#    # 6. add internal internal functionality for direct memory access
#    set_target_properties(
#            karafuto_core PROPERTIES LINK_FLAGS
#            "--bind \
#            -O0 \
#            -s ENVIRONMENT=web \
#            -s ALLOW_MEMORY_GROWTH=1 \
#            -s 'EXPORTED_RUNTIME_METHODS=[\"getValue\"]'"
#    )
#    target_link_libraries(karafuto_core glm)
#endif ()