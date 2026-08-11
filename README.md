# FQR — File Transfer Over QR Codes

FQR is a C++20 application for transferring files through QR codes.

The application divides file data into packets, encodes the packets into QR codes, and can decode those QR codes to reconstruct the original file.

## Features

* Encode files into QR codes.
* Decode QR codes back into files.
* Binary-safe file handling.
* File packetization and reconstruction.
* QR encoding and decoding using ZXing-C++.
* C++20 implementation.
* CMake-based build system.
* Native Windows build using Microsoft Visual C++.
* Linux/macOS support through `pkg-config`.
* Automatic ZXing-C++ acquisition on Windows through CMake `FetchContent`.

---

# Project Structure

```text
fqr/
├── CMakeLists.txt
├── build-win.bat
├── include/
│   ├── decoder.hpp
│   ├── encoder.hpp
│   ├── file_io.hpp
│   ├── file_packet.hpp
│   └── qr.hpp
├── src/
│   ├── main.cpp
│   ├── file_io.cpp
│   ├── file_packet.cpp
│   ├── qr.cpp
│   ├── decoder.cpp
│   └── encoder.cpp
└── README.md
```

---

# Requirements

## Windows

The Windows build uses:

* Visual Studio 18 Community
* MSVC x64 compiler
* CMake 3.20+
* Ninja
* Git
* C++20

ZXing-C++ does **not** need to be manually installed.

CMake downloads and builds ZXing-C++ automatically using `FetchContent`.

The build script expects Visual Studio at:

```text
C:\Program Files\Microsoft Visual Studio\18\Community\
```

If Visual Studio is installed somewhere else, update `build-win.bat`.

---

## Linux / macOS

The Unix build uses:

* C++20 compiler
* CMake 3.20+
* Ninja
* pkg-config
* ZXing-C++

ZXing-C++ must be available through `pkg-config`.

---

# Building on Windows

The easiest way to build FQR on Windows is using the included:

```text
build-win.bat
```

Open a **Developer Command Prompt** or a normal Command Prompt with the required tools available and run:

```bat
build-win.bat
```

The script performs the following steps:

1. Initializes the MSVC x64 build environment.
2. Adds Git to `PATH`.
3. Configures CMake with Ninja.
4. Downloads ZXing-C++ if necessary.
5. Builds ZXing-C++.
6. Builds FQR.

The relevant command is:

```bat
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -S .
```

Then:

```bat
cmake --build build
```

After a successful build, the executable will be located at:

```text
build\fqr.exe
```

---

# Windows Build Process

The Windows build uses CMake's `FetchContent` mechanism to obtain ZXing-C++ directly from its repository.

The project currently uses:

```text
ZXing-C++ v3.1.1
```

CMake downloads the source from:

```text
https://github.com/zxing-cpp/zxing-cpp.git
```

and builds it as part of the FQR project.

This means there is no requirement to install ZXing separately on Windows.

The project also disables unnecessary ZXing components such as:

* Examples
* Qt examples
* Blackbox tests
* Unit tests
* .NET bindings
* Go bindings
* Python module
* C API
* Experimental API

while keeping the required reader and writer functionality enabled.

---

# Building on Linux

Install the required dependencies.

For Arch Linux:

```bash
sudo pacman -S cmake ninja gcc pkgconf zxing-cpp
```

Configure:

```bash
cmake -S . \
    -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release
```

Build:

```bash
cmake --build build -j$(nproc)
```

The executable will be:

```text
build/fqr
```

---

# Running FQR

FQR uses command-line arguments to select between encoding and decoding.

The general syntax is:

```text
fqr -e <file>
fqr -d <directory> <extension>
```

The first argument must be either:

* `-e` — encode a file
* `-d` — decode QR codes

---

## Encode

To encode a file:

### Linux / macOS

```bash
./build/fqr -e "path/to/file"
```

Example:

```bash
./build/fqr -e src/test.png
```

### Windows

```bat
build\fqr.exe -e "path\to\file"
```

Example:

```bat
build\fqr.exe -e "C:\Users\Ali\Pictures\test.png"
```

The file extension is automatically detected from the input file.

For example:

```bash
./build/fqr -e image.png
```

automatically uses:

```text
.png
```

as the file type.

No additional extension argument is required when encoding.

---

## Decode

To decode QR codes:

### Linux / macOS

```bash
./build/fqr -d "path/to/qr_codes" ".extension"
```

Example:

```bash
./build/fqr -d "./qr_codes" ".png"
```

### Windows

```bat
build\fqr.exe -d "path\to\qr_codes" ".extension"
```

Example:

```bat
build\fqr.exe -d ".\qr_codes" ".png"
```

The extension argument tells FQR what extension to use for the reconstructed file.

For example:

```bash
./build/fqr -d ./qr_codes .png
```

produces a reconstructed file using:

```text
.png
```

---

# Command-Line Arguments

| Command | Arguments                 | Description                                               |
| ------- | ------------------------- | --------------------------------------------------------- |
| `-e`    | `<file>`                  | Encode the specified file into QR codes                   |
| `-d`    | `<directory> <extension>` | Decode QR codes from a directory and reconstruct the file |

### Examples

Encode:

```bash
./build/fqr -e image.png
```

Decode:

```bash
./build/fqr -d ./qr_codes .png
```

Windows:

```bat
build\fqr.exe -e image.png
build\fqr.exe -d .\qr_codes .png
```

---

# Invalid Usage

The program requires an operation and its required arguments.

These are invalid:

```bash
./build/fqr
```

```bash
./build/fqr image.png
```

```bash
./build/fqr -x image.png
```

```bash
./build/fqr -d ./qr_codes
```

The program will print the usage information and exit with an error.

---

# CMake Configuration

The project requires CMake 3.20 or newer.

The project is configured for C++20:

```cmake
cmake_minimum_required(VERSION 3.20)

project(fqr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

The executable contains:

```text
src/main.cpp
src/file_io.cpp
src/file_packet.cpp
src/qr.cpp
src/decoder.cpp
src/encoder.cpp
```

Project headers are located under:

```text
include/
```

---

# ZXing-C++

FQR uses **ZXing-C++** for QR-code generation and decoding.

The dependency is handled differently depending on the platform.

### Windows

Windows uses CMake `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    zxing
    GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
    GIT_TAG        v3.1.1
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(zxing)
```

The application links against:

```cmake
ZXing::ZXing
```

### Linux / macOS

Unix systems use `pkg-config`:

```cmake
find_package(PkgConfig REQUIRED)

pkg_check_modules(ZXING REQUIRED zxing)
```

The discovered ZXing libraries and include directories are then linked to FQR.

---

# Application Architecture

The application is separated into several components.

```text
                FQR
                 │
        ┌────────┴────────┐
        │                 │
     ENCODE            DECODE
        │                 │
        ▼                 ▼
    File I/O          QR Reader
        │                 │
        ▼                 ▼
 File Packets        File Packets
        │                 │
        ▼                 ▼
   QR Encoder       File Reconstruction
        │
        ▼
    QR Codes
```

## File I/O

`file_io.cpp`

Responsible for reading files and writing reconstructed data.

## File Packets

`file_packet.cpp`

Handles the packet representation used to divide file data into QR-transmittable pieces.

## Encoder

`encoder.cpp`

Handles the encoding pipeline from file data into QR-code data.

## Decoder

`decoder.cpp`

Handles decoding QR data and reconstructing the original file.

## QR Layer

`qr.cpp`

Provides the QR-code functionality backed by ZXing-C++.

## Main

`main.cpp`

Provides the application's command-line entry point and connects the different components.

---

# Development Build

For a debug build on Windows:

```bat
cmake -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -S .
cmake --build build-debug
```

For Linux:

```bash
cmake -S . \
    -B build-debug \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug

cmake --build build-debug
```

Compilation commands are exported by CMake:

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

This generates:

```text
build/compile_commands.json
```

---

# Cleaning the Build

### Windows

```bat
rmdir /s /q build
```

Then rebuild:

```bat
build-win.bat
```

### Linux / macOS

```bash
rm -rf build
```

Then configure and build again.

---

# Dependencies

| Dependency      | Purpose                   |
| --------------- | ------------------------- |
| C++20           | Application language      |
| CMake 3.20+     | Build system              |
| Ninja           | Build backend             |
| ZXing-C++ 3.1.1 | QR encoding/decoding      |
| Git             | Fetching ZXing on Windows |
| MSVC            | Windows compiler          |
| pkg-config      | Unix dependency discovery |

---

# Platform Support

| Platform | Compiler  | ZXing Dependency   |
| -------- | --------- | ------------------ |
| Windows  | MSVC      | CMake FetchContent |
| Linux    | GCC/Clang | pkg-config         |
| macOS    | Clang     | pkg-config         |

Windows is built **natively with MSVC**. It is not a MinGW or cross-compiled build.

---

# Current Status

FQR is currently under active development.

The project currently contains:

* File I/O
* File packetization
* QR handling
* Encoder
* Decoder
* CMake build system
* Native Windows/MSVC build
* Linux/macOS dependency detection
* Automatic ZXing-C++ acquisition on Windows

---

# License

License information has not yet been specified for this project.
