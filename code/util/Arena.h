#if !defined(ARENA_H)
/* ========================================================================
   $File: Arena.h $
   $Date: October 19 2024 04:51 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define ARENA_H
#include "../Intrinsics.h"
#include <stdlib.h>
#include <string.h>

typedef size_t memory_index;

struct memory_block
{
    void  *MemoryBlock;
    uint8 *BlockOffset; 

    memory_index BlockSize;
};

struct memory_arena
{
    memory_index  Capacity;
    memory_index  Used;
    uint8        *Base;

    int32 ScratchCount;
};

struct scratch_memory
{
    memory_arena *Arena;
    memory_index  Used;
};

// TODO(Sleepster): Add the ability to "free" the memory of the arena
#define PushSize(Arena, size, ...)                 PushSize_(Arena, size * sizeof(uint8), ##__VA_ARGS__)
#define PushStruct(Arena, type, ...)       (type *)PushSize_(Arena, sizeof(type), ##__VA_ARGS__)
#define PushArray(Arena, type, Count, ...) (type *)PushSize_(Arena, sizeof(type) * (Count), ##__VA_ARGS__)

internal inline memory_index
GetAlignmentOffset(memory_arena *Arena, memory_index Alignment = 4)
{
    memory_index Offset = (memory_index)Arena->Base + Arena->Used;
    memory_index AlignmentMask = Alignment - 1;

    memory_index AlignmentOffset = 0;
    if(Offset & AlignmentMask) // If the memory is misaligned
    {
        AlignmentOffset = Alignment - (Offset & AlignmentMask); // align it 
    }

    return(AlignmentOffset);
}

internal inline memory_index
ArenaGetRemainingSize(memory_arena *Arena, memory_index Alignment)
{
    memory_index Result = Arena->Capacity - (Arena->Used + GetAlignmentOffset(Arena, Alignment));
    return(Result);
}

internal void*
PushSize_(memory_arena *Arena, memory_index Size, memory_index Alignment = 4)
{
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Alignment);
    Size += AlignmentOffset;

    Assert((Arena->Used + Size) <= Arena->Capacity);

    void *Result = (void *)(Arena->Base + Arena->Used + AlignmentOffset);
    Arena->Used += Size;

    return(Result);
}

// TODO(Sleepster): Make this simply take a memory offset rather than a memory block
internal inline void 
InitializeArena(memory_arena *Arena, memory_index Capacity, memory_block *BlockBuffer)
{
    Arena->Capacity     = Capacity;
    Arena->Used         = 0;
    Arena->Base         = (uint8 *)BlockBuffer->BlockOffset;
    Arena->ScratchCount = 0;

    BlockBuffer->BlockOffset += Arena->Capacity;
}

internal inline memory_arena
InitSubArena(memory_arena *Arena, memory_index Capacity, memory_index Alignment = 4)
{
    memory_arena Result = {};
    Result.Capacity = Capacity;
    Result.Base = (uint8 *)PushSize_(Arena, Capacity, Alignment);

    return(Result);
}

internal inline scratch_memory
BeginScratchBlock(memory_arena *Arena)
{
    scratch_memory Result;
    Result.Arena = Arena;
    Result.Used  = Arena->Used;

    ++Arena->ScratchCount;
    return(Result);
}

internal inline void
EndScratchBlock(scratch_memory *Scratch)
{
    memory_arena *Arena = Scratch->Arena;
    Assert(Arena->Used >= Scratch->Used);
    Assert(Arena->ScratchCount > 0);

    Arena->Used = Scratch->Used;
    Arena->ScratchCount--;
}

internal inline void
ClearArena(memory_arena *Arena)
{
    Arena->Used = 0;
}

#endif // ARENA_H
