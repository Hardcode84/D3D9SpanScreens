project(D3D9SpanScreens)
cmake_minimum_required(VERSION 2.8.6)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fno-rtti -pedantic -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++ -DINITGUID")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Ofast -flto -ffast-math -funroll-loops -mtune=native")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--kill-at")
endif()

set(SOURCE
    libmain.cpp
    d3d_proxy.cpp
    d3ddevice_proxy.cpp
    d3dswapchain_proxy.cpp
    settings.cpp
    )

set(HEADERS
    libmain.hpp
    d3d_proxy.hpp
    d3ddevice_proxy.hpp
    d3dswapchain_proxy.hpp
    settings.hpp
    )

add_subdirectory("${PROJECT_SOURCE_DIR}/simple_logger")
add_subdirectory("${PROJECT_SOURCE_DIR}/tools")

#add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
include(GenerateExportHeader)
add_compiler_export_flags()
add_library(d3d9 SHARED ${SOURCE} ${HEADERS})

if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(d3d9 libdxerr9.a)
    target_link_libraries(d3d9 d3dx9.a)
endif()

generate_export_header(d3d9
        EXPORT_FILE_NAME "${PROJECT_SOURCE_DIR}/d3d9_export.h")

