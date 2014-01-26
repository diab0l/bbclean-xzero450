rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

mkdir _projects.vs11.32_xp
cd _projects.vs11.32_xp
cmake -G "Visual Studio 11" -T "v110_xp" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32_xp ..
if %errorlevel% neq 0 goto TERM
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
if %errorlevel% neq 0 goto TERM
devenv BlackBoxZero.sln /build  RelWithDebInfo /project INSTALL
if %errorlevel% neq 0 goto TERM

cd ..

mkdir _projects.vs11.32_xp.dbg
cd _projects.vs11.32_xp.dbg
cmake -G "Visual Studio 11" -T "v110_xp" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32_xp.dbg ..
if %errorlevel% neq 0 goto TERM
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
if %errorlevel% neq 0 goto TERM
devenv BlackBoxZero.sln /build Debug /project INSTALL
if %errorlevel% neq 0 goto TERM

rem devenv BlackBoxZero.sln [options] solutionconfig /project bbLeanSkin32
rem devenv BlackBoxZero.sln [options] solutionconfig [/project projectnameorfile [/projectconfig name]]
goto NOPAUSE

:TERM
pause

:NOPAUSE
