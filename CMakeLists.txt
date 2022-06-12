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
        add_definitions(-DVULKAN_BACKEND)
    elseif (APPLE)
        set(PLATFORM "MacOS")

#        set(BACKEND "Vulkan")
#        add_definitions(-DVULKAN_BACKEND)

        set(BACKEND "OpenCL")
        add_definitions(-DOPENCL_BACKEND)
    elseif (UNIX)
        set(PLATFORM "Linux")

        set(BACKEND "Vulkan")
        add_definitions(-DVULKAN_BACKEND)

#        set(BACKEND "OpenCL")
#        add_definitions(-DOPENCL_BACKEND)
    elseif (WIN32)
        set(PLATFORM "Windows")

        #        set(BACKEND "Vulkan")
        #        add_definitions(-DVULKAN_BACKEND)

        set(BACKEND "OpenCL")
        add_definitions(-DOPENCL_BACKEND)
    endif ()
endif ()

message("🖥️ Current platform: ${PLATFORM}")
message("🖼️ Current backend: ${BACKEND} \n")

# Exclude everything platform-dependent
if (ON)
    set(CPP_HEADERS_ADDITIONALS "")
    file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
            ${MAIN_SOURCE_DIR}/layer/presenters/*.hpp
            ${MAIN_SOURCE_DIR}/layer/presenters/*.inl)

    set(CPP_SOURCES_ADDITIONALS "")
    file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS ${MAIN_SOURCE_DIR}/layer/presenters/*.cpp)

    list(REMOVE_ITEM CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
    list(REMOVE_ITEM CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
endif ()

# Create temporary variables contain platform-dependent sources
if (ON)
    message("🖊️ Grab dependent sources")

    if (${BACKEND} STREQUAL "Vulkan")
        message("\t - Vulkan adapter")
        set(CPP_HEADERS_ADDITIONALS "")
        file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
                ${MAIN_SOURCE_DIR}/layer/presenters/vulkan/*.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/vulkan/*.inl
                ${MAIN_SOURCE_DIR}/layer/presenters/debug/*.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/debug/*.inl
                ${MAIN_SOURCE_DIR}/layer/presenters/one-to-one/*.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/ImageResult.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/IRenderContext.hpp)

        set(CPP_SOURCES_ADDITIONALS "")
        file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
                ${MAIN_SOURCE_DIR}/layer/presenters/vulkan/*.cpp
                ${MAIN_SOURCE_DIR}/layer/presenters/debug/*.cpp
                ${MAIN_SOURCE_DIR}/layer/presenters/one-to-one/*.cpp
                ${MAIN_SOURCE_DIR}/layer/presenters/IRenderContext.cpp)

        list(APPEND CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
        list(APPEND CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
    elseif (${BACKEND} STREQUAL "OpenCL")
        message("\t - OpenCL adapter")
        set(CPP_HEADERS_ADDITIONALS "")
        file(GLOB_RECURSE CPP_HEADERS_ADDITIONALS
                ${MAIN_SOURCE_DIR}/layer/presenters/opencl/*.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/opencl/*.inl
                ${MAIN_SOURCE_DIR}/layer/presenters/debug/*.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/debug/*.inl
                ${MAIN_SOURCE_DIR}/layer/presenters/one-to-one/*.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/ImageResult.hpp
                ${MAIN_SOURCE_DIR}/layer/presenters/IRenderContext.hpp)

        set(CPP_SOURCES_ADDITIONALS "")
        file(GLOB_RECURSE CPP_SOURCES_ADDITIONALS
                ${MAIN_SOURCE_DIR}/layer/presenters/opencl/*.cpp
                ${MAIN_SOURCE_DIR}/layer/presenters/debug/*.cpp
                ${MAIN_SOURCE_DIR}/layer/presenters/one-to-one/*.cpp
                ${MAIN_SOURCE_DIR}/layer/presenters/IRenderContext.cpp)

        list(APPEND CPP_HEADERS ${CPP_HEADERS_ADDITIONALS})
        list(APPEND CPP_SOURCES ${CPP_SOURCES_ADDITIONALS})
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

    if (${PLATFORM} STREQUAL "MacOS")
        if (${BACKEND} STREQUAL "Vulkan")
            message("\t - Added Vulkan")
            find_package(Vulkan REQUIRED)
        elseif (${BACKEND} STREQUAL "OpenCL")
            message("\t - Added OpenCL")
            find_package(OpenCL REQUIRED)
        endif ()
    endif ()

    if (${PLATFORM} STREQUAL "Linux")
        if (${BACKEND} STREQUAL "Vulkan")
            message("\t - Added Vulkan")
            find_package(Vulkan REQUIRED)
        elseif (${BACKEND} STREQUAL "OpenCL")
            message("\t - Added OpenCL")
            find_package(OpenCL REQUIRED)
        endif ()
    endif ()
endif ()

# Declare targets
if (ON)
    message("")
    message("🎯 Declare targets")

    if (${PLATFORM} STREQUAL "Android")
        add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
    else ()
        add_library(libkcore SHARED ${CPP_HEADERS} ${CPP_SOURCES})
        add_executable(karafuto_core ${CPP_HEADERS} ${CPP_SOURCES})
    endif ()

    message("\t - Link libraries")
    if (ON)
        if (${PLATFORM} STREQUAL "Android")
            if (${BACKEND} STREQUAL "Vulkan")
                message("\t - Vulkan libraries")
                set(LIBRARIES glm HTTPRequest vulkan)
                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
            endif ()
        elseif (${PLATFORM} STREQUAL "Windows")
            if (${BACKEND} STREQUAL "Vulkan")
                message("\t - Vulkan libraries")
                set(LIBRARIES glm HTTPRequest wsock32 ws2_32)

                find_package(Vulkan REQUIRED)

                target_link_libraries(libkcore PUBLIC ${LIBRARIES} ${Vulkan_LIBRARIES})
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES} ${Vulkan_LIBRARIES})
            elseif (${BACKEND} STREQUAL "OpenCL")
                message("\t - OpenCL libraries")
                set(LIBRARIES opencl glm HTTPRequest wsock32 ws2_32)

                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
                target_link_directories(libkcore PUBLIC ${LIBRARIES} ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
                target_link_directories(karafuto_core PUBLIC ${LIBRARIES} ${OCL_ROOT}/lib/x86_64 ${OCL_ROOT}/lib/x86)
            endif ()
        elseif (${PLATFORM} STREQUAL "Linux")
            if (${BACKEND} STREQUAL "Vulkan")
                message("\t - OpenCL libraries")
                set(LIBRARIES glm HTTPRequest vulkan)

                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
            elseif (${BACKEND} STREQUAL "OpenCL")
                message("\t - OpenCL libraries")
                set(LIBRARIES glm HTTPRequest)

                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
                target_link_directories(libkcore PUBLIC ${OpenCL_LIBRARY})
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
                target_link_directories(karafuto_core PUBLIC ${OpenCL_LIBRARY})
            endif ()
        elseif (${PLATFORM} STREQUAL "MacOS")
            if (${BACKEND} STREQUAL "Vulkan")
                message("\t - Vulkan libraries")
                set(LIBRARIES glm HTTPRequest Vulkan::Vulkan)

                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
            elseif (${BACKEND} STREQUAL "OpenCL")
                message("\t - OpenCL libraries")
                set(LIBRARIES glm HTTPRequest "-framework OpenCL")

                target_link_libraries(libkcore PUBLIC ${LIBRARIES})
                target_link_libraries(karafuto_core PUBLIC ${LIBRARIES})
            endif ()
        endif ()
    endif ()

    message("\t - Add headers")
    if (ON)
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
            target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
        elseif (${PLATFORM} STREQUAL "MacOS")
            set(INCLUDE_COMPOUND
                    ${THIRD_PARTY_DIR}/rapidjson/include
                    ${THIRD_PARTY_DIR}/lrucache17
                    ${THIRD_PARTY_DIR}/earcut/include
                    ${THIRD_PARTY_DIR}/stb-image
                    )
            target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
            target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
        elseif (${PLATFORM} STREQUAL "Linux")
            set(INCLUDE_COMPOUND
                    ${THIRD_PARTY_DIR}/rapidjson/include
                    ${THIRD_PARTY_DIR}/lrucache17
                    ${THIRD_PARTY_DIR}/earcut/include
                    ${THIRD_PARTY_DIR}/stb-image
                    )
            target_include_directories(libkcore PUBLIC ${INCLUDE_COMPOUND})
            target_include_directories(karafuto_core PUBLIC ${INCLUDE_COMPOUND})
        endif ()
    endif ()
endif ()
