@echo off
rem Configure + build everything (core, CLI, tests, Qt GUI).
setlocal
call "%~dp0msvc-env.bat" cmake -S "%~dp0.." -B "%~dp0..\build" -G Ninja -DBUILD_GUI=ON -DBUILD_CLI=ON -DBUILD_TESTS=ON || exit /b 1
call "%~dp0msvc-env.bat" cmake --build "%~dp0..\build" || exit /b 1
