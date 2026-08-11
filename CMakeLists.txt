cmake_minimum_required(VERSION 3.20)

project(fqr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(fqr
    src/main.cpp
    src/file_io.cpp
    src/file_packet.cpp
    src/qr.cpp
    src/decoder.cpp
    src/encoder.cpp
    src/crypto.cpp
)

target_include_directories(fqr
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(WIN32)
    # ------------------------------------------------------------------
    # Windows: build ZXing-C++ from source via CMake FetchContent.
    # pkg-config is a Unix-only tool; FetchContent is the normal
    # Windows alternative that requires no vcpkg installation.
    # ------------------------------------------------------------------
    include(FetchContent)

    # Minimise the zxing-cpp build footprint — we only need the
    # reader and QR writer/encoder.
    set(ZXING_EXAMPLES          OFF CACHE BOOL   "" FORCE)
    set(ZXING_EXAMPLES_QT       OFF CACHE BOOL   "" FORCE)
    set(ZXING_BLACKBOX_TESTS    OFF CACHE BOOL   "" FORCE)
    set(ZXING_UNIT_TESTS        OFF CACHE BOOL   "" FORCE)
    set(ZXING_TEST_DOTNET       OFF CACHE BOOL   "" FORCE)
    set(ZXING_TEST_GO           OFF CACHE BOOL   "" FORCE)
    set(ZXING_TEST_INSTALL      OFF CACHE BOOL   "" FORCE)
    set(ZXING_PYTHON_MODULE     OFF CACHE BOOL   "" FORCE)
    set(ZXING_C_API             OFF CACHE BOOL   "" FORCE)
    set(ZXING_EXPERIMENTAL_API  OFF CACHE BOOL   "" FORCE)
    set(ZXING_WRITERS           ON  CACHE STRING "" FORCE)
    set(ZXING_READERS           ON  CACHE BOOL   "" FORCE)

    FetchContent_Declare(
        zxing
        GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
        GIT_TAG        v3.1.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(zxing)

    # ------------------------------------------------------------------
    # zxing-cpp headers live in core/src/ but consumer code includes
    # them as <ZXing/...>.  Re-create the installed layout so the
    # existing #include paths resolve correctly.
    # ------------------------------------------------------------------
    set(ZXING_PUBLIC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_zxing_includes/ZXing")
    file(MAKE_DIRECTORY "${ZXING_PUBLIC_INCLUDE_DIR}")
    file(GLOB _zxing_src_headers "${zxing_SOURCE_DIR}/core/src/*.h")
    file(GLOB _zxing_gen_headers "${zxing_BINARY_DIR}/core/*.h")
    file(COPY ${_zxing_src_headers} DESTINATION "${ZXING_PUBLIC_INCLUDE_DIR}")
    file(COPY ${_zxing_gen_headers} DESTINATION "${ZXING_PUBLIC_INCLUDE_DIR}")
    target_include_directories(fqr PRIVATE "${CMAKE_BINARY_DIR}/_zxing_includes")

    target_link_libraries(fqr
        PRIVATE
            ZXing::ZXing
            Sodium::Sodium
    )

    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
    find_package(Sodium REQUIRED)
else()
    # ------------------------------------------------------------------
    # Unix (Linux / macOS): use pkg-config to locate ZXing-C++.
    # ------------------------------------------------------------------
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(ZXING REQUIRED zxing)

    target_include_directories(fqr
        PRIVATE
            ${ZXING_INCLUDE_DIRS}
    )

    target_link_directories(fqr
        PRIVATE
            ${ZXING_LIBRARY_DIRS}
    )

    target_link_libraries(fqr
        PRIVATE
            ${ZXING_LIBRARIES}
            Sodium::Sodium
    )

    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
    find_package(Sodium REQUIRED)
endif()
