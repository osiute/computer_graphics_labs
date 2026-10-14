cmake_minimum_required(VERSION 3.10)
project(lab14_horizon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../common_utils ../libraries)

add_executable(lab14_app
    main.cpp
    horizon.cpp
    solids.cpp
    roberts.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
    ../common_utils/geometry.cpp
    ../common_utils/obj_io.cpp
)
