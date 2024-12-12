#if !defined(SORTING_H)
/* ========================================================================
   $File: Sorting.h $
   $Date: Wed, 27 Nov 24: 01:13PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define SORTING_H
#include "../Intrinsics.h"

internal void
RadixSort(void *PrimaryBuffer, void *SortingBuffer, int32 ItemCount, int32 ItemSize, int32 OffsetOfValue, int32 ValueSizeInBits)
{
    // NOTE(Sleepster): Hey look local_persist can be good! 
    // This prevents the value from being reinitialized every time we reenter the function. We instead set it once, since it's constant
    local_persist const int32 Radix = 256;
    local_persist const int32 BitsPerPass = 8;

    const int32 PassCount = (ValueSizeInBits + BitsPerPass -1) / BitsPerPass;
	const int64 HalfRangeOfValue = 1ULL << (ValueSizeInBits - 1);

    int64 Count[Radix];
    int64 DigitSum[Radix];

    for(int32 PassIndex = 0;
        PassIndex < PassCount;
        ++PassIndex)
    {
        int32 BitShift = PassIndex * BitsPerPass;

        memset(Count, 0, sizeof(Count));
		for(int32 Index = 0;
            Index < ItemCount;
            ++Index)
        {
            uint8 *Item = (uint8 *)PrimaryBuffer + Index * ItemSize;
            uint64 ValueToSort = *(uint64 *)(Item + OffsetOfValue);
            ValueToSort += HalfRangeOfValue;

            uint32 Digit = (ValueToSort >> BitShift) & (Radix - 1);
            ++Count[Digit];
        }

        DigitSum[0] = 0;
        for(uint32 Index = 1;
            Index < Radix;
            ++Index)
        {
            DigitSum[Index] = DigitSum[Index - 1] + Count[Index - 1]; 
        }

        for(int32 Index = 0;
            Index < ItemCount;
            ++Index)
        {
            uint8 *Item = (uint8 *)PrimaryBuffer + Index * ItemSize;
            uint64 ValueToSort = *(uint64 *)(Item + OffsetOfValue);
            ValueToSort += HalfRangeOfValue;

            uint32 Digit = (ValueToSort >> BitShift) & (Radix - 1);

            memcpy((uint8 *)SortingBuffer + DigitSum[Digit] * ItemSize, Item, ItemSize);
            ++DigitSum[Digit];
        }
        memcpy(PrimaryBuffer, SortingBuffer, ItemCount * ItemSize);
    }
}

#endif
