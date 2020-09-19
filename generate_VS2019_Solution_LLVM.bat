@echo off

echo ---------------------------------------------------
echo   Nodable - Configure Visual Studio 2019 Solution.
echo ---------------------------------------------------

set BUILD_FOLDER=build
set VS_STUDIO_VERION="Visual Studio 16 2019"
set PLATFORM_TOOLSET="LLVM"


IF NOT EXIST build GOTO :build

:choice
echo Warning: the target folder "%BUILD_FOLDER%" exists. Do you want to continue ?
set /P c= (ignore (Enter), delete(D), quit(N) [Y/N/Q]) 
if /I "%c%" EQU "D" goto :deleteFolder
if /I "%c%" EQU "" goto :build
if /I "%c%" EQU "N" goto :abort
goto :choice

:deleteFolder
rmdir /S /Q %BUILD_FOLDER%
mkdir %BUILD_FOLDER%

:build
cmake -G %VS_STUDIO_VERION% -B %BUILD_FOLDER% -T %PLATFORM_TOOLSET% || goto :error

:done
echo Finished! Open the VS solution into "./%BUILD_FOLDER%"
IF "%1"=="--no-prompt" exit
pause
exit

:abort
echo Configuration canceled.
pause
exit

:error
echo Configuration failed: Unable to generate Solution.
pause