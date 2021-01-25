#!/bin/bash

make -j4
rm *.o
make CXX=x86_64-w64-mingw32-g++ CXXFLAGS=-static-libstdc++ LIBS="-lws2_32 -lraylib -lwinmm" TARGET=windows/main.exe
rm *.o
