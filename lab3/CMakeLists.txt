cmake_minimum_required(VERSION 3.10)
project(PentagramLab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(pentagram_app
    main.cpp
    geometry.cpp
    svg_handler.cpp
    canvas.cpp
    builtin_draw.cpp
    dda_draw.cpp
    bresenham_draw.cpp
    integer_bresenham_draw.cpp
)

target_link_libraries(pentagram_app PRIVATE Threads::Threads)