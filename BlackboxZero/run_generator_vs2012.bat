rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

echo 64bit...
mkdir _projects.vs11.64
cd _projects.vs11.64
cmake -G "Visual Studio 11 Win64" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs64 ..
if %errorlevel% neq 0 goto TERM
echo "generated projects are in ./_projects.vs11.64"

cd ..

echo 32bit...
mkdir _projects.vs11.32
cd _projects.vs11.32
cmake -G "Visual Studio 11" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32 ..
if %errorlevel% neq 0 goto TERM
echo "generated projects are in ./_projects.vs11.32"

goto NOPAUSE

:TERM
pause

:NOPAUSE
