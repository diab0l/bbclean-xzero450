mkdir _projects.mingw.64
cd _projects.mingw.64
set MYOLDPATH=%PATH%
setlocal
set PATH=c:\\mingw64\\bin;%PATH%
cmake -G "MSYS Makefiles" ..
rem make Debug
rem devenv BlackBoxZero.sln /build Debug /project INSTALL
rem devenv BlackBoxZero.sln /build ReleaseWithDebugInfo

cd ..
mkdir _projects.mingw.32
set PATH=c:\\mingw\\bin;%PATH%
cd _projects.mingw.32
cmake -G "MSYS Makefiles" ..
rem devenv BlackBoxZero.sln [options] solutionconfig /project bbLeanSkin32
rem devenv BlackBoxZero.sln [options] solutionconfig [/project projectnameorfile [/projectconfig name]]
pause
