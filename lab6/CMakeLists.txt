cmake_minimum_required(VERSION 3.10)
project(lab6_geom)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../common_utils)

add_executable(lab6_app
    main.cpp
    image_io.cpp
    geom.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)
