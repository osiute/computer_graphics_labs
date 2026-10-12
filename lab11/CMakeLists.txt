cmake_minimum_required(VERSION 3.10)
project(lab11_projections)

set(CMAKE_CXX_STANDARD 17)

# Подключаем общие утилиты из соседней папки common_utils
include_directories(../common_utils)

add_executable(lab11_app
    main.cpp
    platonic.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)