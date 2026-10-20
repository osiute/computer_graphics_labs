cmake_minimum_required(VERSION 3.16)
project(lab16_materials LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lab16_core
    obj_mtl.cpp
    triangulation.cpp
    renderer.cpp
    png_io.cpp
    ../common_utils/geometry.cpp
)
target_include_directories(lab16_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common_utils"
)
target_include_directories(lab16_core SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../libraries")
if(MSVC)
    target_compile_options(lab16_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(lab16_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(lab16_app main.cpp)
target_link_libraries(lab16_app PRIVATE lab16_core)
if(WIN32)
    target_link_libraries(lab16_app PRIVATE shell32)
endif()
target_compile_definitions(lab16_app PRIVATE LAB16_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

include(CTest)
if(BUILD_TESTING)
    add_executable(lab16_tests tests.cpp)
    target_link_libraries(lab16_tests PRIVATE lab16_core)
    target_include_directories(lab16_tests SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../libraries")
    target_compile_definitions(lab16_tests PRIVATE LAB16_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME lab16_regression COMMAND lab16_tests)
endif()

if(MSVC)
    target_compile_options(lab16_app PRIVATE /W4 /utf-8)
    if(BUILD_TESTING)
        target_compile_options(lab16_tests PRIVATE /W4 /utf-8)
    endif()
else()
    target_compile_options(lab16_app PRIVATE -Wall -Wextra -Wpedantic)
    if(BUILD_TESTING)
        target_compile_options(lab16_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
