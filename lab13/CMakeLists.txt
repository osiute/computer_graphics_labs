cmake_minimum_required(VERSION 3.12)
project(Lab13_PolygonClipping)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common_utils)

set(COMMON_SOURCES
    ../common_utils/image.cpp
    ../common_utils/netpbm.cpp
    ../common_utils/geometry.cpp
)

set(LAB13_SOURCES
    main.cpp
    clipper.cpp
    filler.cpp
    parser.cpp
)

add_executable(lab13_clipping ${LAB13_SOURCES} ${COMMON_SOURCES})