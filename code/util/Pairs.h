#if !defined(PAIRS_H)
/* ========================================================================
   $File: Pairs.h $
   $Date: October 03 2024 06:24 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define PAIRS_H

#include "../Intrinsics.h"

template <typename Type1, typename Type2>
struct pair
{
    Type1 First;
    Type2 Second;
    
    pair() : First(Type1()), Second(Type2()) {}
    
    pair(const Type1 &A, const Type2 &B) 
        : First(A), Second(B) {}
    
    
    bool32 operator==(const pair &Comparitor)
    {
        return(First == Comparitor.First && Second == Comparitor.Second);
    }
    
    bool32 operator!=(const pair &Comparitor)
    {
        return(!(*this == Comparitor));
    }
};

template <typename Type1, typename Type2>
internal inline pair<Type1, Type2> 
MakePair(const Type1 &A, const Type2 &B)
{
    return(pair<Type1, Type2>(A, B));
}

#endif // PAIRS_H
