@echo off
REM this is used instead of CMake, MakeGNU, Ninja, etc. Build tools are used to optimize the compiling of programmes. Large projects can take long to compile.

mkdir ..\build
pushd ..\build
clang++ ..\code\win32_entry.cpp -lgdi32 -o win32_entry.exe
popd