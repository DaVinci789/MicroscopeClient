#!/bin/bash

g++ main.cpp -g -lraylib -o main
x86_64-w64-mingw32-g++ main.cpp -static-libstdc++ -lwinpthread -lraylib -lwinmm -lkernel32 -lntdll -lws2_32 -o windows/main.exe
