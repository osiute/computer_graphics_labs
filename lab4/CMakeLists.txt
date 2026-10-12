cmake_minimum_required(VERSION 3.10)
project(lab4_circles)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../common_utils)

add_executable(lab4_app
    main.cpp
    image_io.cpp
    draw_utils.cpp
    pentagon.cpp
    equation_draw.cpp
    parametric_draw.cpp
    bresenham_circle.cpp
    builtin_circle.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)
