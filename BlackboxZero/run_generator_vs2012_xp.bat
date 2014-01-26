rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

echo 32bit...
mkdir _projects.vs11.32
cd _projects.vs11.32
cmake -G "Visual Studio 11" -DCMAKE_VS_PLATFORM_TOOLSET=v110_xp -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.vs32 ..
if %errorlevel% neq 0 goto TERM
echo "generated projects are in ./_projects.vs11.32"

goto NOPAUSE

:TERM
pause

:NOPAUSE
