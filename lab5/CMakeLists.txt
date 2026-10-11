cmake_minimum_required(VERSION 3.10)
project(PixelTransformLab)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Перечисляем все модули нашего проекта
add_executable(pixel_app
    main.cpp
    image.cpp
    netpbm.cpp
    filters.cpp
)