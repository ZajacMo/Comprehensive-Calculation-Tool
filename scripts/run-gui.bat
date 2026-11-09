@echo off
rem Launch the GUI and capture a screenshot of its window.
setlocal
set "QTDIR=C:\Qt\6.6.2\msvc2019_64"
set "PATH=%QTDIR%\bin;%PATH%"
cd /d "%~dp0.."
start "" "build\calc_gui.exe"
timeout /t 3 /nobreak >nul
echo LAUNCHED
