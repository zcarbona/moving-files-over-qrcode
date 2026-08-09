cmake_minimum_required(VERSION 3.20)

project(fqr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find pkg-config
find_package(PkgConfig REQUIRED)

# Find ZXing-C++
pkg_check_modules(ZXING REQUIRED zxing)

add_executable(fqr
    src/main.cpp
    src/file_io.cpp
    src/file_packet.cpp
    src/qr.cpp
)

target_include_directories(fqr
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${ZXING_INCLUDE_DIRS}
)

target_link_directories(fqr
    PRIVATE
        ${ZXING_LIBRARY_DIRS}
)

target_link_libraries(fqr
    PRIVATE
        ${ZXING_LIBRARIES}
)