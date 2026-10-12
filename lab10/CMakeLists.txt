cmake_minimum_required(VERSION 3.10)
project(lab10_curves)

set(CMAKE_CXX_STANDARD 17)

# Подключаем общие утилиты из папки common_utils
include_directories(../common_utils)

add_executable(lab10_app
    main.cpp
    curves.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)