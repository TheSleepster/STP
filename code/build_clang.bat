@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts= 

REM -Og -g
Set CommonCompilerFlags=-std=c++20 -fpermissive -ffast-math -fno-exceptions -fno-rtti -O3 -march=native -flto -fno-exceptions -fno-rtti -DNDEBUG -fuse-ld=lld -fms-runtime-lib=mt
Set CommonLinkerFlags=-lopengl32 -lkernel32 -luser32 -lshell32 -lgdi32 -lwinmm -lmsvcrt -l../data/deps/raylib/lib/raylib.lib -l../data/deps/yyjson/lib/yyjson.lib
Set CommonIncludes=-I"../data/deps" -I"../data/deps/raylib/include" -I"../data/deps/yyjson/include/"

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
clang++ %opts% ../code/STP_Entry.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -oSTP.exe
popd
