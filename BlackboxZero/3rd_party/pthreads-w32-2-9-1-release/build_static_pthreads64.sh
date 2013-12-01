#!/bin/bash
cd pthreads.2/
make clean GC-static
mingw_w64_x86_64_prefix=/c/mingw64
cp -v libpthreadGC2.a $mingw_w64_x86_64_prefix/lib/libpthread.a
cp -v pthread.h sched.h semaphore.h $mingw_w64_x86_64_prefix/include
