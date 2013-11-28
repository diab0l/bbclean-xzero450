mkdir _projects.mingw.64
cd _projects.mingw.64
export PATH=/c/mingw64/bin:$PATH
cmake -G "MSYS Makefiles" ..
make

cd ..
mkdir _projects.mingw.32
export PATH=/c/mingw/bin:$PATH
cd _projects.mingw.32
cmake -G "MSYS Makefiles" ..
make
