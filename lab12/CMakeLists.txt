cmake_minimum_required(VERSION 3.10)
project(lab12_surfaces)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../common_utils ../libraries)

add_executable(lab12_app
    main.cpp
    curves.cpp
    surfaces.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
    ../common_utils/geometry.cpp
    ../common_utils/obj_io.cpp
)
