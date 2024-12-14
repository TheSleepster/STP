@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts= 

Set CommonCompilerFlags=-std=c++20 -fpermissive -ffast-math -fno-exceptions -fno-rtti -Og -g -Wall -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers -Wno-deprecated-declarations -Wno-missing-braces -Wno-unused-but-set-variable
Set CommonLinkerFlags=-lopengl32 -lkernel32 -luser32 -lshell32 -lgdi32 -lwinmm -lmsvcrt -l../data/deps/raylib/lib/raylib.lib
Set CommonIncludes=-I"../data/deps" -I"../data/deps/raylib/include" -I"../data/deps/yyjson/include/"

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
clang++ %opts% ../code/STP_Entry.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -oSTP.exe
popd
