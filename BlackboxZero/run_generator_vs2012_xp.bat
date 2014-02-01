rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

echo 32bit...
mkdir _projects.vs11.32_xp
cd _projects.vs11.32_xp
cmake -G "Visual Studio 11" -T "v110_xp" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32_xp ..
if %errorlevel% neq 0 goto TERM
echo "generated projects are in ./_projects.vs11.32_xp"

cd ..

echo 64bit...
mkdir _projects.vs11.64_xp
cd _projects.vs11.64_xp
cmake -G "Visual Studio 11 Win64" -T "v110_xp" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs64_xp ..
if %errorlevel% neq 0 goto TERM
echo "generated projects are in ./_projects.vs11.64_xp"

cd ..

goto NOPAUSE

:TERM
pause

:NOPAUSE
