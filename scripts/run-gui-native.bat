@echo off
rem Run the GUI smoke test against the real Windows platform (needs a desktop session).
setlocal
set "QTDIR=C:\Qt\6.6.2\msvc2019_64"
set "PATH=%QTDIR%\bin;%PATH%"
cd /d "%~dp0.."
build\gui_smoke.exe "%~dp0..\build\screenshots-native"
echo EXITCODE=%ERRORLEVEL%
