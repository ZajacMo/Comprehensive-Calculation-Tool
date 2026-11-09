@echo off
cd /d "%~dp0.."
build\calc_tests.exe
echo EXITCODE=%ERRORLEVEL%
