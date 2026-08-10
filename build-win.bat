@echo off
setlocal

REM Set up MSVC build environment for x64
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Add Git to PATH for CMake FetchContent
set PATH=C:\Program Files\Git\bin;%PATH%

REM Configure with Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -S .
if errorlevel 1 exit /b 1

REM Build
cmake --build build
if errorlevel 1 exit /b 1

echo Build complete!
