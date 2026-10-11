cmake_minimum_required(VERSION 3.10)
project(lab7_filters)

set(CMAKE_CXX_STANDARD 17)

# Указываем компилятору, где искать общие заголовки
include_directories(../common_utils)

# Собираем исполняемый файл, включая общие модули
add_executable(lab7_app
    main.cpp
    filters_lab7.cpp
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
)