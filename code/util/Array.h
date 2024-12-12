#if !defined(ARRAY_H)
/* ========================================================================
   $File: Array.h $
   $Date: October 19 2024 05:03 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define ARRAY_H

#include "../Intrinsics.h"
#include "Arena.h"

// NOTE(Sleepster): Just a test, C++ makes me vomit
template <typename Type, int32 Capacity>
struct array
{
    int32 Count = 0;
    int32 Size  = Capacity; 
    Type  Data[Capacity];

    inline void
    MakeHeap(memory_arena *Memory)
    {
        Data = (Type *)ArenaAlloc(Memory, sizeof(Type) * Capacity);
        for(int Index = 0; Index < Capacity; ++Index)
        {
            Type Element = Data[Index];
            memset(&Element, 0, (sizeof(Type)));
        }
    }

    int32 
    Add(Type Element)
    {
        Check(Count >= 0, "Invalid Count\n");
        Check(Count < Capacity, "Array Full\n");

        Data[Count] = Element;
        return(++Count);
    }

    inline void
    Remove(int32 Index)
    {
        Check(Index >= 0, "Invalid Index\n");
        Check(Index < Count, "Index Is Invalid\n");

        Data[Index] = Data[--Count];
    }

    inline void
    Clear()
    {
        Count = 0;
    }

    inline bool32
    Full()
    {
        return(Count >= Capacity);
    }

    inline int64 
    SizeInBytes()
    {
        int64 Size = Capacity * sizeof(Type);
        return(Size);
    }

    inline Type& 
    operator[](int32 Index)
    {
        Check(Index >= 0, "Invalid Index\n");
        Check(Index < Capacity, "Invalid Index\n");

        return(Data[Index]);
    }
};

#endif // ARRAY_H

