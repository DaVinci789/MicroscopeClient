#!/bin/bash

make -j4 CXXFLAGS=-O3
rm *.o
make CXX=x86_64-w64-mingw32-g++ CXXFLAGS=-"static-libstdc++ -lcomdlg32 -lole32 -O3 -fpermissive" LIBS="-lws2_32 -lraylib -lwinmm" TARGET=windows/main.exe -j4
rm *.o
