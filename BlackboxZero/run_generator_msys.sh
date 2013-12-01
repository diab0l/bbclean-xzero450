#!/bin/bash
# this file is intended for usage in MSYS terminal

echo 32bit...
mkdir _projects.msys.32
cd _projects.msys.32
export PATH=/c/mingw/bin:$PATH
cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.msys32 ..

cd ..

echo 64bit...
mkdir _projects.msys.64
cd _projects.msys.64
export PATH=/c/mingw64/bin:$PATH
cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.msys64 ..


