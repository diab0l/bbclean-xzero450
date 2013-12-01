#!/bin/bash
# this file is intended for usage in MSYS terminal

echo 32bit...
mkdir _projects.mingw.32
export PATH=/c/mingw/bin:$PATH
cd _projects.mingw.32
cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.mingw32 ..

cd ..

echo 64bit...
mkdir _projects.mingw.64
cd _projects.mingw.64
export PATH=/c/mingw64/bin:$PATH
cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX:PATH=c:/bbZero.mingw64 ..


