mkdir _projects.vs11.64
cd _projects.vs11.64
cmake -G "Visual Studio 11 Win64" ..
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86_amd64
devenv BlackBoxZero.sln /build Debug
rem devenv BlackBoxZero.sln /build Debug /project INSTALL
rem devenv BlackBoxZero.sln /build ReleaseWithDebugInfo

cd ..
mkdir _projects.vs11.32
cd _projects.vs11.32
cmake -G "Visual Studio 11" ..
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
devenv BlackBoxZero.sln /build Debug
rem devenv BlackBoxZero.sln [options] solutionconfig /project bbLeanSkin32
rem devenv BlackBoxZero.sln [options] solutionconfig [/project projectnameorfile [/projectconfig name]]
pause
