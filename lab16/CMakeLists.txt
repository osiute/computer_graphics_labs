cmake_minimum_required(VERSION 3.16)
project(lab16_materials LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lab16_app
    main.cpp
    obj_mtl.cpp
    triangulation.cpp
    renderer.cpp
    png_io.cpp
    ../common_utils/geometry.cpp
)
target_include_directories(lab16_app PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common_utils"
)
target_include_directories(lab16_app SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../libraries")
if(MSVC)
    target_compile_options(lab16_app PRIVATE /W4 /utf-8)
else()
    target_compile_options(lab16_app PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    target_link_libraries(lab16_app PRIVATE shell32)
endif()
target_compile_definitions(lab16_app PRIVATE LAB16_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
