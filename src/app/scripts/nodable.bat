@echo off
SET NDBL_DIR=%~dp0
SET LOG_PREFIX=[MSG-Launcher]
echo %LOG_PREFIX% nodable base dir is: %NDBL_DIR:~0,-1%
echo %LOG_PREFIX% Running nodable.exe ...
start /wait /d %NDBL_DIR% %NDBL_DIR%bin\nodable.exe
echo %LOG_PREFIX% nodable.exe stopped.