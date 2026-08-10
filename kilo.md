# KILO.md — Project Build & Automation Instructions

## 1. Project Overview

This is a C++ project that was originally developed and built on Linux.

It encodes arbitrary files into a sequence of QR-code PNG images (PGM format)
and decodes them back, enabling file transfer over QR codes.

The current project structure is:

```text
.
├── include/
│   ├── file_io.hpp
│   ├── file_packet.hpp
│   └── qr.hpp
│
├── src/
│   ├── app
│   ├── decode_main.cpp
│   ├── file_io.cpp
│   ├── file_packet.cpp
│   ├── main.cpp
│   ├── qr.cpp
│   └── test.png
│
├── .gitignore
├── CMakeLists.txt
├── build-win.bat      # Windows build helper
└── kilo.md
```

## 2. Dependencies

| Dependency   | Linux (pkg-config) | Windows (FetchContent) |
|---|---|---|
| CMake ≥ 3.20 | system package | bundled with VS or standalone |
| C++20 compiler | GCC / Clang | MSVC (Visual Studio 2022+) |
| ZXing-C++ ≥ 3.1.1 | `libzxing-cpp-dev` | auto-downloaded via CMake FetchContent |
| pkg-config | system package | **not required** on Windows |

## 3. Building on Linux (original platform)

```bash
# Install dependencies (Debian/Ubuntu example)
sudo apt install cmake g++ pkg-config libzxing-cpp-dev

# Configure & build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The resulting binary is `build/fqr`.

## 4. Building on Windows

Requirements:
- Visual Studio 2022 or newer with the **Desktop development with C++**
  workload (provides MSVC and the C++ build tools).
- Git (bundled with Visual Studio or installed separately).
- C++20 support (MSVC 14.31+ / `/std:c++20`).

CMake and Ninja are bundled with Visual Studio.  A convenience script
**`build-win.bat`** automates the full process.

### Quick start

```bat
build-win.bat
```

This script:
1. Initialises the MSVC x64 environment (`vcvars64.bat`).
2. Adds Git to `PATH` (required by `FetchContent`).
3. Configures the project with CMake + Ninja.
4. Builds the `fqr` executable.

The output binary is `build\fqr.exe`.

### Manual build

Open a **Developer Command Prompt for VS** and run:

```bat
:: Configure (downloads and builds zxing-cpp automatically)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

:: Build
cmake --build build
```

### How dependencies are resolved on Windows

The original `CMakeLists.txt` used `pkg_check_modules(ZXING REQUIRED zxing)`
which is a Unix-only mechanism.  On Windows the `CMakeLists.txt` takes an
`if (WIN32)` branch that uses **CMake `FetchContent`** to clone and build
[ZXing-C++](https://github.com/zxing-cpp/zxing-cpp) (`v3.1.1`) from source
as part of the project build — no vcpkg installation is required.

Because `FetchContent` places headers in the build tree root (`core/src/`)
rather than under the installed `ZXing/` prefix, the `CMakeLists.txt` also
copies the public headers into a synthetic `ZXing/` include directory so
that existing `#include <ZXing/...>` directives resolve correctly.

## 5. Usage

```text
Usage:
  fqr -e <file>        Encode a file into QR codes (qr_0000.pgm, qr_0001.pgm, ...)
  fqr -d <directory>   Decode all .pgm QR codes in a directory back to a file

Examples:
  fqr -e image.png
  fqr -d ./qr_codes
```

Round-trip verification performed:
- `test.txt` (51 bytes, 1 chunk) — encode → decode → byte-identical ✓
- `src/test.png` (31 495 bytes, 16 chunks) — encode → decode → byte-identical ✓
