#if !defined(INTRINSICS_H)
/* ========================================================================
   $File: Intrinsics.h $
   $Date: October 19 2024 04:46 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define INTRINSICS_H
#if CLOVER_SLOW 

#ifdef _WIN64
#define Check(Expression, Message, ...) if(!(Expression)) {char CHECKBUFFER[1024] = {}; sprintf(CHECKBUFFER, Message, __VA_ARGS__); printf("%s\n", CHECKBUFFER); fflush(stdout); __debugbreak();}
#define Assert(Expression) if(!(Expression)) {__debugbreak();}
#define Trace(Message) {printm(Message)}
#define printm(Message, ...)   {char BUFFER[128]  = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER); fflush(stdout);}
#define printlm(Message, ...)  {char BUFFER[5192] = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER); fflush(stdout);}
#define cl_Error(Message, ...) {char BUFFER[512]  = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("[ERROR]: %s\n", BUFFER);  fflush(stdout); Assert(0);}
#define cl_Info(Message, ...)  {char BUFFER[512]  = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("[INFO]: %s\n", BUFFER); fflush(stdout);}
#define InvalidCodePath __debugbreak()

#elif __linux__
#include <csignal>
#define Check(Expression, Message, ...) if(!(Expression)) {char CHECKBUFFER[1024] = {}; sprintf(CHECKBUFFER, Message, ##__VA_ARGS__); printf("%s\n", CHECKBUFFER); raise(SIGTRAP); fflush(stdout);}
#define Assert(Expression) if(!(Expression)) {raise(SIGTRAP);}
#define Trace(Message) {printm(Message)}
#define printm(Message, ...)   {char BUFFER[128]  = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER);}
#define printlm(Message, ...)  {char BUFFER[5192] = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER);}
#define cl_Error(Message, ...) {char BUFFER[512]  = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("[ERROR]: %s\n", BUFFER); Assert(0);}
#define cl_Info(Message, ...)  {char BUFFER[512]  = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "[ERROR]: String is too large\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("[INFO]: %s\n", BUFFER);}
#define InvalidCodePath raise(SIGTRAP)

#elif __APPLE__
#define Assert(Expression)
#define Check(Expression, Message, ...)
#define Trace(Message)
#define printm(Message, ...)
#define printlm(Message, ...)
#define cl_Error(Message, ...)
#define cl_Info(Message, ...) 
#define InvalidCodePath
#endif

#else

#define Assert(Expression)
#define Check(Expression, Message, ...)
#define Trace(Message)
#define printm(Message, ...)
#define printlm(Message, ...)
#define cl_Error(Message, ...)
#define cl_Info(Message, ...) 
#define InvalidCodePath

#endif

#define Kilobytes(Value) ((uint64)(Value) * 1024)
#define Megabytes(Value) ((uint64)Kilobytes(Value) * 1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value) * 1024)

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define global_variable static
#define local_persist   static
#define internal        static

#define external extern "C"

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef int8     bool8;
typedef int32    bool32;

typedef float    real32;
typedef double   real64;

#define FIRST_ARG(arg1, ...) arg1
#define SECOND_ARG(arg1, arg2, ...) arg2

#if _MSC_VER
#define alignas(x)       __declspec(align(x))
#define inline           __forceinline

// TODO(Sleepster): Double check this stuff
#define WriteBarrier     _WriteBarrier(); _mm_sfence()
#define ReadBarrier      _ReadBarrier();  _mm_sfence()
#define ReadWriteBarrier _ReadWriteBarrier(); _mm_lfence()

#include <intrin.h>
inline int32 AtomicCompareExchangei32(int32 volatile *Target, int32 Expected, int32 Value)
{
    int32 Result = _InterlockedCompareExchange((long *)Target, Expected, Value);
    return(Result);
}

#else

#define alignas(x)       alignas()
#define inline           inline

#define WriteBarrier     __atomic_signal_fence(__ATOMIC_RELEASE); __atomic_thread_fence(__ATOMIC_RELEASE)
#define ReadBarrier      __atomic_signal_fence(__ATOMIC_ACQUIRE); __atomic_thread_fence(__ATOMIC_ACQUIRE)
#define ReadWriteBarrier __atomic_signal_fence(__ATOMIC_ACQ_REL); __atomic_thread_fence(__ATOMIC_ACQ_REL)

#endif

#endif // INTRINSICS_H

