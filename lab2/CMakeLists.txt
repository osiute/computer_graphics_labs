cmake_minimum_required(VERSION 3.10)
project(lab2_compose)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../common_utils)

add_executable(lab2_app
    main.cpp
    lab2_ops.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)
