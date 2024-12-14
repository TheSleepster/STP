@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts=-DCLOVER_SLOW=1

Set CommonCompilerFlags=-W4 -std:c++20 -permissive -fp:fast -Fm -GR- -EHa- -Od -Oi -Zi -wd4996 -wd4100 -wd4505
Set CommonLinkerFlags=-ignore:4098 -incremental:no opengl32.lib kernel32.lib user32.lib shell32.lib gdi32.lib winmm.lib msvcrt.lib "../data/deps/raylib/lib/raylib.lib" -NODEFAULTLIB:LIBCMT
Set CommonIncludes=-I"../data/deps" -I"../data/deps/raylib/include" -I"../data/deps/yyjson/include/"

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl %opts% ../code/STP_Entry.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -OUT:"STP.exe" -PDB:STP.pdb
popd
