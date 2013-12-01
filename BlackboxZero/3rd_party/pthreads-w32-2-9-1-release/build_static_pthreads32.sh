#!/bin/bash
cd pthreads.2/
make clean GC-static
mingw_prefix=/c/MinGW
cp -v libpthreadGC2.a $mingw_prefix/lib/libpthread.a
cp -v pthread.h sched.h semaphore.h $mingw_prefix/include
