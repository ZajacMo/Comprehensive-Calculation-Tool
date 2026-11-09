@echo off
rem Start the web server in the background on a fixed test port.
setlocal
set "QTDIR=C:\Qt\6.6.2\msvc2019_64"
cd /d "%~dp0.."
start "" /b "build\calc_server.exe" --port 8099 --no-browser
timeout /t 2 /nobreak >nul
echo SERVER_STARTED
