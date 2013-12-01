rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
rem there should be no sh.exe in your path!
@echo off

echo 32bit...
mkdir _projects.eclipse.32
cd _projects.eclipse.32
set PATH=c:\\mingw\\bin;%PATH%
cmake -G "Eclipse CDT4 - MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.eclipse32 ..
if %errorlevel% neq 0 goto TERM

cd ..

echo 64bit...
mkdir _projects.eclipse.64
cd _projects.eclipse.64
set PATH=c:\\mingw64\\bin;%PATH%
cmake -G "Eclipse CDT4 - MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.eclipse64 ..
if %errorlevel% neq 0 goto TERM

cd ..

goto NOPAUSE

:TERM
pause

:NOPAUSE
