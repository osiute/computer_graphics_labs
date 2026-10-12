cmake_minimum_required(VERSION 3.10)
project(lab9_clipper)

set(CMAKE_CXX_STANDARD 17)

# Подключаем общие утилиты из соседней папки
include_directories(../common_utils)

add_executable(lab9_app
    main.cpp
    clipper.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)