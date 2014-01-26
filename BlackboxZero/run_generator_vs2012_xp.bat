rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

echo 32bit...
mkdir _projects.vs11.32_xp
cd _projects.vs11.32_xp
cmake -G "Visual Studio 11" -T "v110_xp" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32 ..
if %errorlevel% neq 0 goto TERM
echo "generated projects are in ./_projects.vs11.32_xp"

goto NOPAUSE

:TERM
pause

:NOPAUSE
