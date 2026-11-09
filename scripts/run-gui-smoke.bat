@echo off
rem Run the offscreen GUI smoke test and write screenshots into build\screenshots.
setlocal
set "QTDIR=C:\Qt\6.6.2\msvc2019_64"
set "PATH=%QTDIR%\bin;%PATH%"
set "QT_QPA_PLATFORM=offscreen"
cd /d "%~dp0.."
build\gui_smoke.exe "%~dp0..\build\screenshots"
echo EXITCODE=%ERRORLEVEL%
