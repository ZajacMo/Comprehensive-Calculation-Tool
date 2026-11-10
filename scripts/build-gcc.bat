@echo off
rem Build the whole project with MinGW GCC to catch portability issues that
rem MSVC hides (MSVC transitively includes headers GCC does not).
setlocal
set "MINGW=C:\Qt\Tools\mingw1120_64"
set "PATH=%MINGW%\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;%PATH%"
cd /d "%~dp0.."
cmake -S . -B build-gcc -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1120_64/bin/g++.exe ^
  -DCMAKE_C_COMPILER=C:/Qt/Tools/mingw1120_64/bin/gcc.exe ^
  -DBUILD_GUI=OFF -DBUILD_CLI=ON -DBUILD_SERVER=ON -DBUILD_TESTS=ON || exit /b 1
cmake --build build-gcc || exit /b 1
