cmake_minimum_required(VERSION 3.10)
project(lab8_interp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../common_utils)

add_executable(lab8_app
    main.cpp
    image_io.cpp
    interp.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)
