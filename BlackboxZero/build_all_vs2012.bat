rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

mkdir _projects.vs11.64
cd _projects.vs11.64
cmake -G "Visual Studio 11 Win64" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs64 ..
if %errorlevel% neq 0 goto TERM
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86_amd64
if %errorlevel% neq 0 goto TERM
devenv BlackBoxZero.sln /build RelWithDebInfo
if %errorlevel% neq 0 goto TERM
rem devenv BlackBoxZero.sln /build Debug /project INSTALL
rem devenv BlackBoxZero.sln /build ReleaseWithDebugInfo

cd ..

mkdir _projects.vs11.64.dbg
cd _projects.vs11.64.dbg
cmake -G "Visual Studio 11 Win64" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs64.dbg ..
if %errorlevel% neq 0 goto TERM
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86_amd64
if %errorlevel% neq 0 goto TERM
devenv BlackBoxZero.sln /build Debug
if %errorlevel% neq 0 goto TERM

cd ..

mkdir _projects.vs11.32
cd _projects.vs11.32
cmake -G "Visual Studio 11" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32 ..
if %errorlevel% neq 0 goto TERM
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
if %errorlevel% neq 0 goto TERM
devenv BlackBoxZero.sln /build  RelWithDebInfo
if %errorlevel% neq 0 goto TERM

cd ..

mkdir _projects.vs11.32.dbg
cd _projects.vs11.32.dbg
cmake -G "Visual Studio 11" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32.dbg ..
if %errorlevel% neq 0 goto TERM
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
if %errorlevel% neq 0 goto TERM
devenv BlackBoxZero.sln /build Debug
if %errorlevel% neq 0 goto TERM

rem devenv BlackBoxZero.sln [options] solutionconfig /project bbLeanSkin32
rem devenv BlackBoxZero.sln [options] solutionconfig [/project projectnameorfile [/projectconfig name]]
goto NOPAUSE

:TERM
pause

:NOPAUSE
