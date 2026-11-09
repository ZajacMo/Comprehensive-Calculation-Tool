@echo off
cd /d "%~dp0.."
build\calc_cli.exe %*
echo EXITCODE=%ERRORLEVEL%
