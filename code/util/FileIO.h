#if !defined(FILEIO_H)
/* ========================================================================
   $File: FileIO.h $
   $Date: October 20 2024 04:39 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define FILEIO_H

#include "../Intrinsics.h"
#include "Arena.h"
#include "String.h"

#include <time.h>
#include <sys/stat.h>

typedef time_t filetime;

internal int32
GetFileSizeInBytes(string Filepath)
{
    int32 FileSize = 0;
    FILE *File = fopen((const char *)Filepath.Data, "rb");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);
        fclose(File);
    }
    else
    {
        cl_Error("Failure to find the designated file!\n");
        FileSize = 0;
    }
    return(FileSize);
}

internal char *
ReadEntireFile(string Filepath, uint32 Size, char *Buffer)
{
    Check(Filepath.Data != nullptr, "Cannot find the file designated!\n");
    Check(Buffer != nullptr, "Provide a valid buffer!\n");
    Check(Size >= 0, "Size is less than 0!\n");
    
    FILE *File = fopen((const char *)Filepath.Data, "rb");
    if(File)
    {
        memset(Buffer, 0, Size + 1);
        fread(Buffer, sizeof(char), Size, File);

        fclose(File);
    }
    else
    {
        cl_Error("Failure to find the File!\n");
    }
    return(Buffer);
}

internal string
ReadEntireFileMA(memory_arena *ArenaAllocator, string Filepath)
{
    string File = {};
    int32 FileSize = GetFileSizeInBytes(Filepath);
    Check(FileSize >= 0, "FileSize is less than 0!\n");
    
    if(FileSize)
    {
        char *Buffer = (char *)PushSize(ArenaAllocator, uint64(FileSize + 1));
        memset(Buffer, 0, FileSize + 1);
        File.Data = (uint8 *)ReadEntireFile(Filepath, FileSize, Buffer);
        File.Data[FileSize + 1] = 0;
        File.Length = FileSize;
    }
    else
    {
        cl_Error("File size is 0, File is either invalid or you provided the wrong path.\n");
    }
    
    return(File);
}

internal time_t
FileGetLastWriteTime(string Filepath)
{
    struct stat FileStats;
    stat(CSTR(Filepath), &FileStats);

    return(FileStats.st_mtime);
}

internal inline bool32
CloverCompareFiletime(filetime A, filetime B)
{
    return(A == B);
}

#endif // FILEIO_H

