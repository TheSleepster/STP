/*
  THIS IS A FORK OF HANDMADE MATH

  HandmadeMath.h v2.0.0

  This is a single header file with a bunch of useful types and functions for
  games and graphics. Consider it a lightweight alternative to GLM that works
  both C and C++.

  =============================================================================
  CONFIG
  =============================================================================

  By default, all angles in Handmade Math are specified in radians. However, it
  can be configured to use degrees or turns instead. Use one of the following
  defines to specify the default unit for angles:

    #define HANDMADE_MATH_USE_RADIANS
    #define HANDMADE_MATH_USE_DEGREES
    #define HANDMADE_MATH_USE_TURNS

  Regardless of the default angle, you can use the following functions to
  specify an angle in a particular unit:

    AngleRad(radians)
    AngleDeg(degrees)
    AngleTurn(turns)

  The definitions of these functions change depending on the default unit.

  -----------------------------------------------------------------------------

  Handmade Math ships with SSE (SIMD) implementations of several common
  operations. To disable the use of SSE intrinsics, you must define
  HANDMADE_MATH_NO_SSE before including this file:

    #define HANDMADE_MATH_NO_SSE
    #include "HandmadeMath.h"

  -----------------------------------------------------------------------------

  To use Handmade Math without the C runtime library, you must provide your own
  implementations of basic math functions. Otherwise, HandmadeMath.h will use
  the runtime library implementation of these functions.

  Define HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS and provide your own
  implementations of SINF, COSF, TANF, ACOSF, and SQRTF
  before including HandmadeMath.h, like so:

    #define HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS
    #define SINF MySinF
    #define COSF MyCosF
    #define TANF MyTanF
    #define ACOSF MyACosF
    #define SQRTF MySqrtF
    #include "HandmadeMath.h"
  
  By default, it is assumed that your math functions take radians. To use
  different units, you must define ANGLE_USER_TO_INTERNAL and
  ANGLE_INTERNAL_TO_USER. For example, if you want to use degrees in your
  code but your math functions use turns:

    #define ANGLE_USER_TO_INTERNAL(a) ((a)*DegToTurn)
    #define ANGLE_INTERNAL_TO_USER(a) ((a)*TurnToDeg)

  =============================================================================
  
  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  =============================================================================

  CREDITS

  Originally written by Zakary Strange.

  Functionality:
   Zakary Strange (strangezak@protonmail.com && @strangezak)
   Matt Mascarenhas (@miblo_)
   Aleph
   FieryDrake (@fierydrake)
   Gingerbill (@TheGingerBill)
   Ben Visness (@bvisness)
   Trinton Bullard (@Peliex_Dev)
   @AntonDan
   Logan Forman (@dev_dwarf)

  Fixes:
   Jeroen van Rijn (@J_vanRijn)
   Kiljacken (@Kiljacken)
   Insofaras (@insofaras)
   Daniel Gibson (@DanielGibson)
*/

#ifndef HANDMADE_MATH_H
#define HANDMADE_MATH_H

/* let's figure out if SSE is really available (unless disabled anyway)
   (it isn't on non-x86/x86_64 platforms or even x86 without explicit SSE support)
   => only use "#ifdef HANDMADE_MATH__USE_SSE" to check for SSE support below this block! */
#ifndef HANDMADE_MATH_NO_SSE
# ifdef _MSC_VER /* MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2) */
#  if defined(_M_AMD64) || ( defined(_M_IX86_FP) && _M_IX86_FP >= 1 )
#   define HANDMADE_MATH__USE_SSE 1
#  endif
# else /* not MSVC, probably GCC, clang, icc or something that doesn't support SSE anyway */
#  ifdef __SSE__ /* they #define __SSE__ if it's supported */
#   define HANDMADE_MATH__USE_SSE 1
#  endif /*  __SSE__ */
# endif /* not _MSC_VER */
#endif /* #ifndef HANDMADE_MATH_NO_SSE */

#if (!defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
# define HANDMADE_MATH__USE_C11_GENERICS 1
#endif

#ifdef HANDMADE_MATH__USE_SSE
# include <xmmintrin.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wfloat-equal"
# if (defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 8)) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wmissing-braces"
# endif
# ifdef __clang__
#  pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# endif
#endif

#if defined(__GNUC__) || defined(__clang__)
# define DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
# define DEPRECATED(msg) __declspec(deprecated(msg))
#else
# define DEPRECATED(msg)
#endif

#if !defined(HANDMADE_MATH_USE_DEGREES) \
&& !defined(HANDMADE_MATH_USE_TURNS) \
&& !defined(HANDMADE_MATH_USE_RADIANS)
# define HANDMADE_MATH_USE_RADIANS
#endif

#define PI32 3.14159265359f
#define DEG180 180.0
#define DEG18032 180.0f
#define TURNHALF 0.5
#define TURNHALF32 0.5f
#define RadToDeg ((float)(DEG180/PI))
#define RadToTurn ((float)(TURNHALF/PI))
#define DegToRad ((float)(PI/DEG180))
#define DegToTurn ((float)(TURNHALF/DEG180))
#define TurnToRad ((float)(PI/TURNHALF))
#define TurnToDeg ((float)(DEG180/TURNHALF))

#if defined(HANDMADE_MATH_USE_RADIANS)
# define AngleRad(a) (a)
# define AngleDeg(a) ((a)*DegToRad)
# define AngleTurn(a) ((a)*TurnToRad)
#elif defined(HANDMADE_MATH_USE_DEGREES)
# define AngleRad(a) ((a)*RadToDeg)
# define AngleDeg(a) (a)
# define AngleTurn(a) ((a)*TurnToDeg)
#elif defined(HANDMADE_MATH_USE_TURNS)
# define AngleRad(a) ((a)*RadToTurn)
# define AngleDeg(a) ((a)*DegToTurn)
# define AngleTurn(a) (a)
#endif

#if !defined(HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS)
# include <math.h>
# define SINF sinf
# define COSF cosf
# define TANF tanf
# define SQRTF sqrtf
# define ACOSF acosf
#endif

#if !defined(ANGLE_USER_TO_INTERNAL)
# define ANGLE_USER_TO_INTERNAL(a) (DegreeToRad(a))
#endif

#if !defined(ANGLE_INTERNAL_TO_USER)
# if defined(HANDMADE_MATH_USE_RADIANS)
#  define ANGLE_INTERNAL_TO_USER(a) (a) 
# elif defined(HANDMADE_MATH_USE_DEGREES)
#  define ANGLE_INTERNAL_TO_USER(a) ((a)*RadToDeg)
# elif defined(HANDMADE_MATH_USE_TURNS)
#  define ANGLE_INTERNAL_TO_USER(a) ((a)*RadToTurn)
# endif
#endif

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define ABS(a) ((a) > 0 ? (a) : -(a))
#define Modulous(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define Square(x) ((x) * (x))
#define Sign(x)   ((x >= 0) ? 1 : -1)

typedef union vec2
{
    struct
    {
        float X, Y;
    };
    
    struct
    {
        float U, V;
    };
    
    struct
    {
        float Left, Right;
    };
    
    struct
    {
        float Width, Height;
    };
    
    float Elements[2];
    
#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
    
#endif
} vec2;

typedef union ivec2
{
    struct 
    {
        int X;
        int Y;
    };
    struct 
    {
        int Width;
        int Height;
    };

    int Elements[2];
    
#ifdef __cplusplus
    inline int &operator[](const int &Index)
    {
        return Elements[Index];
    }
    
#endif
}ivec2;

// OPERATOR OVERLOADING

static inline ivec2
operator-(ivec2 A, ivec2 B)
{
    ivec2 Result = {};
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    
    return(Result);
}

static inline ivec2
operator-(ivec2 A, int B)
{
    ivec2 Result = {0};
    
    Result.X = A.X - B;
    Result.Y = A.Y - B;
    
    return(Result);
}

static inline ivec2
operator+(ivec2 A, ivec2 B)
{
    ivec2 Result = {};
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    
    return(Result);
}

static inline ivec2
operator*(ivec2 A, ivec2 B)
{
    ivec2 Result = {};
    Result.X = A.X * B.X;
    Result.Y = A.Y * B.Y;
    
    return(Result);
}

static inline ivec2
operator*(ivec2 A, int B)
{
    ivec2 Result = {0};
    
    Result.X = int(A.X * B);
    Result.Y = int(A.Y * B);
    
    return(Result);
}

static inline ivec2
operator*(ivec2 A, float B)
{
    ivec2 Result = {};
    Result.X = int(A.X * B);
    Result.Y = int(A.Y * B);
    
    return(Result);
}

static inline ivec2
operator/(ivec2 A, ivec2 B)
{
    ivec2 Result = {};
    Result.X = A.X / B.X;
    Result.Y = A.Y / B.Y;
    
    return(Result);
}

static inline ivec2
operator/(ivec2 A, float B)
{
    ivec2 Result = {};
    
    Result.X = int(A.X * B);
    Result.Y = int(A.Y * B);
    
    return(Result);
}

static inline ivec2
operator*=(ivec2 &A, int B)
{
    A.X = A.X * B;
    A.Y = A.Y * B;
    return(A);
}

static inline vec2
v2Cast(ivec2 A)
{
    vec2 Result = {};
    
    Result.X = float(A.X);
    Result.Y = float(A.Y);
    
    return(Result);
}

static inline ivec2
iv2Cast(vec2 A)
{
    ivec2 Result = {};
    
    Result.X = int(A.X);
    Result.Y = int(A.Y);
    
    return(Result);
}

struct range_r32
{
    float Minimum;
    float Maximum;
};

struct range_v2
{
    vec2 Min;
    vec2 Max;
};

typedef union vec3
{
    struct
    {
        float X, Y, Z;
    };
    
    struct
    {
        float U, V, W;
    };
    
    struct
    {
        float R, G, B;
    };
    
    struct
    {
        vec2 XY;
        float _Ignored0;
    };
    
    struct
    {
        float _Ignored1;
        vec2 YZ;
    };
    
    struct
    {
        vec2 UV;
        float _Ignored2;
    };
    
    struct
    {
        float _Ignored3;
        vec2 VW;
    };
    
    float Elements[3];
    
#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif
} vec3;

static inline vec3
v2Expand(vec2 A, float B)
{
    vec3 Result = {};
    Result.X = A.X;
    Result.Y = A.Y;
    Result.Z = B;
    
    return(Result);
}

typedef union
{
    int Elements[3];
    
    struct
    {
        int x;
        int y;
        int z;
    };
    
    struct
    {
        int Width;
        int Height;
        int Channels;
    };
}ivec3;

typedef union vec4
{
    struct
    {
        union
        {
            vec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };
        
        float W;
    };
    struct
    {
        union
        {
            vec3 RGB;
            struct
            {
                float R, G, B;
            };
        };
        
        float A;
    };
    
    struct
    {
        vec2 XY;
        float _Ignored0;
        float _Ignored1;
    };
    
    struct
    {
        float _Ignored2;
        vec2 YZ;
        float _Ignored3;
    };
    
    struct
    {
        float _Ignored4;
        float _Ignored5;
        vec2 ZW;
    };
    
    float Elements[4];
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSE;
#endif
    
#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif
} vec4;

static inline vec4
v2Expand(vec2 A, float B, float C)
{
    vec4 Result = {};
    Result.X = A.X;
    Result.Y = A.Y;
    Result.Z = B;
    Result.W = C;
    
    return(Result);
}

typedef union
{
    int Elements[4];
    struct
    {
        int X;
        int Y;
        int Width;
        int Height;
    };
}ivec4;

typedef union mat2
{
    float Elements[2][2];
    vec2 Columns[2];
    
#ifdef __cplusplus
    inline vec2 &operator[](const int &Index)
    {
        return Columns[Index];
    }
#endif
} mat2;

typedef union mat3
{
    float Elements[3][3];
    vec3 Columns[3];
    
#ifdef __cplusplus
    inline vec3 &operator[](const int &Index)
    {
        return Columns[Index];
    }
#endif
} mat3;

typedef union mat4
{
    float Elements[4][4];
    vec4 Columns[4];
    
#ifdef __cplusplus
    inline vec4 &operator[](const int &Index)
    {
        return Columns[Index];
    }
#endif
} mat4;

typedef union quat
{
    struct
    {
        union
        {
            vec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };
        
        float W;
    };
    
    float Elements[4];
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSE;
#endif
} quat;

typedef signed int Bool;

/*
 * Angle unit conversion functions
 */
static inline float DegreeToRad(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle;
#elif defined(HANDMADE_MATH_USE_DEGREES) 
    float Result = Angle * DegToRad;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle * TurnToRad;
#endif
    
    return Result;
}

static inline float RadiansToDeg(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle * RadToDeg;
#elif defined(HANDMADE_MATH_USE_DEGREES) 
    float Result = Angle;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle * TurnToDeg;
#endif
    
    return Result;
}

static inline float RadianToTurn(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle * RadToTurn;
#elif defined(HANDMADE_MATH_USE_DEGREES) 
    float Result = Angle * DegToTurn;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle;
#endif
    
    return Result;
}

static inline vec4
PositionFromMat4(mat4 M)
{
    vec4 Result = {};
    Result.X = M.Elements[3][0];
    Result.Y = M.Elements[3][1];
    Result.Z = M.Elements[3][2];
    Result.W = M.Elements[3][3];

    return(Result);
}

/*
 * Floating-point math functions
 */

static inline float SinF(float Angle)
{
    return SINF(ANGLE_USER_TO_INTERNAL(Angle));
}

static inline float CosF(float Angle)
{
    return COSF(ANGLE_USER_TO_INTERNAL(Angle));
}

static inline float TanF(float Angle)
{
    return TANF(ANGLE_USER_TO_INTERNAL(Angle));
}

static inline float ACosF(float Arg)
{
    return ANGLE_INTERNAL_TO_USER(ACOSF(Arg));
}

static inline float SqrtF(float Float)
{
    
    float Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 In = _mm_set_ss(Float);
    __m128 Out = _mm_sqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#else
    Result = SQRTF(Float);
#endif
    
    return Result;
}

static inline float InvSqrtf(float Float)
{
    
    float Result;
    
    Result = 1.0f/SqrtF(Float);
    
    return Result;
}


/*
 * Utility functions
 */

static inline float Lerp(float A, float Time, float B)
{
    return (1.0f - Time) * A + Time * B;
}

static inline float Clamp(float Min, float Value, float Max)
{
    
    float Result = Value;
    
    if (Result < Min)
    {
        Result = Min;
    }
    
    if (Result > Max)
    {
        Result = Max;
    }
    
    return Result;
}

static float
r32Clamp(float Value, float Min, float Max)
{
    if (Value <= Min) return Min;
    if (Value >= Max) return Max;
    return Value;
}

static vec2 
v2Clamp(vec2 Value, vec2 Min, vec2 Max)
{
    Value.X = r32Clamp(Value.X, Min.X, Max.X);
    Value.Y = r32Clamp(Value.Y, Min.Y, Max.Y);
    return(Value);
}



static inline bool
Equals(float A, float B, float Tolerance)
{
    return(fabs(A - B) <= Tolerance);
}


static inline void
Approach(float *Value, float Target, float Rate, float Delta_t)
{
    *Value += float((Target - *Value) * (1.0 - pow(2.0f, -Rate * Delta_t)));
    if(Equals(*Value, Target, 0.001f))
    {
        *Value = Target;
    }
}

static inline void
v2Approach(vec2 *Value, vec2 Target, float Rate, float Delta_t)
{
    Approach(&(Value->X), Target.X, Rate, Delta_t);
    Approach(&(Value->Y), Target.Y, Rate, Delta_t);
}


/*
 * Vector initialization
 */

static inline vec2 v2Create(float X, float Y)
{
    
    vec2 Result;
    Result.X = X;
    Result.Y = Y;
    
    return Result;
}

static inline vec3 v3Create(float X, float Y, float Z)
{
    
    vec3 Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    
    return Result;
}

static inline vec4 v4Create(float X, float Y, float Z, float W)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif
    
    return Result;
}

static inline vec4 v3Expand(vec3 Vector, float W)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(Vector.X, Vector.Y, Vector.Z, W);
#else
    Result.XYZ = Vector;
    Result.W = W;
#endif
    
    return Result;
}


/*
 * Binary vector operations
 */

static inline vec2 v2Add(vec2 Left, vec2 Right)
{
    
    vec2 Result;
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    
    return Result;
}

static inline vec3 v3Add(vec3 Left, vec3 Right)
{
    
    vec3 Result;
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    
    return Result;
}

static inline vec4 v4Add(vec4 Left, vec4 Right)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif
    
    return Result;
}

static inline vec2 v2Subtract(vec2 Left, vec2 Right)
{
    
    vec2 Result;
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    
    return Result;
}

static inline vec3 v3Subtract(vec3 Left, vec3 Right)
{
    
    vec3 Result;
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    
    return Result;
}

static inline vec4 v4Subtract(vec4 Left, vec4 Right)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif
    
    return Result;
}

static inline vec2 v2Multiply(vec2 Left, vec2 Right)
{
    
    vec2 Result;
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    
    return Result;
}

static inline vec2 v2Scale(vec2 Left, float Right)
{
    
    vec2 Result;
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    
    return Result;
}

static inline vec3 v3Multiply(vec3 Left, vec3 Right)
{
    
    vec3 Result;
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;
    
    return Result;
}

static inline vec3 v3Scale(vec3 Left, float Right)
{
    
    vec3 Result;
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;
    
    return Result;
}

static inline vec4 v4Multiply(vec4 Left, vec4 Right)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_mul_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;
    Result.W = Left.W * Right.W;
#endif
    
    return Result;
}

static inline vec4 v4Scale(vec4 Left, float Right)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;
    Result.W = Left.W * Right;
#endif
    
    return Result;
}

static inline vec2 v2Divide(vec2 Left, vec2 Right)
{
    
    vec2 Result;
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    
    return Result;
}

static inline vec2 v2Reduce(vec2 Left, float Right)
{
    
    vec2 Result;
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    
    return Result;
}

static inline vec3 v3Divide(vec3 Left, vec3 Right)
{
    
    vec3 Result;
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;
    
    return Result;
}

static inline vec3 v3Reduce(vec3 Left, float Right)
{
    
    vec3 Result;
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;
    
    return Result;
}

static inline vec4 v4Divide(vec4 Left, vec4 Right)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_div_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;
    Result.W = Left.W / Right.W;
#endif
    
    return Result;
}

static inline vec4 v4Reduce(vec4 Left, float Right)
{
    
    vec4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;
    Result.W = Left.W / Right;
#endif
    
    return Result;
}

static inline Bool v2Equal(vec2 Left, vec2 Right)
{
    return Left.X == Right.X && Left.Y == Right.Y;
}

static inline Bool v3Equal(vec3 Left, vec3 Right)
{
    return Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z;
}

static inline Bool v4Equal(vec4 Left, vec4 Right)
{
    return Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z && Left.W == Right.W;
}

static inline float v2Dot(vec2 Left, vec2 Right)
{
    return (Left.X * Right.X) + (Left.Y * Right.Y);
}

static inline float v3Dot(vec3 Left, vec3 Right)
{
    return (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z);
}

static inline float v4Dot(vec4 Left, vec4 Right)
{
    
    float Result;
    
    // NOTE(zak): IN the future if we wanna check what version SSE is support
    // we can use _mm_dp_ps (4.3) but for now we will use the old way.
    // Or a r = _mm_mul_ps(v1, v2), r = _mm_hadd_ps(r, r), r = _mm_hadd_ps(r, r) for SSE3
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = ((Left.X * Right.X) + (Left.Z * Right.Z)) + ((Left.Y * Right.Y) + (Left.W * Right.W));
#endif
    
    return Result;
}

static inline vec3 v3Cross(vec3 Left, vec3 Right)
{
    
    vec3 Result;
    Result.X = (Left.Y * Right.Z) - (Left.Z * Right.Y);
    Result.Y = (Left.Z * Right.X) - (Left.X * Right.Z);
    Result.Z = (Left.X * Right.Y) - (Left.Y * Right.X);
    
    return Result;
}


/*
 * Unary vector operations
 */

static inline float v2LengthSquared(vec2 A)
{
    return v2Dot(A, A);
}

static inline float v3LengthSquared(vec3 A)
{
    return v3Dot(A, A);
}

static inline float v4LengthSquared(vec4 A)
{
    return v4Dot(A, A);
}

static inline float v2Length(vec2 A)
{
    return SqrtF(v2LengthSquared(A));
}

static inline float v2Distance(vec2 A, vec2 B)
{
    return(v2Length(v2Subtract(A, B)));
}

static inline float v3Length(vec3 A)
{
    return SqrtF(v3LengthSquared(A));
}

static inline float v4Length(vec4 A)
{
    return SqrtF(v4LengthSquared(A));
}

static inline vec2 v2Normalize(vec2 A)
{
    return v2Scale(A, InvSqrtf(v2Dot(A, A)));
}

static inline vec3 v3Normalize(vec3 A)
{
    return v3Scale(A, InvSqrtf(v3Dot(A, A)));
}

static inline vec4 v4Normalize(vec4 A)
{
    return v4Scale(A, InvSqrtf(v4Dot(A, A)));
}

/*
 * Utility vector functions
 */

static inline vec2 v2Lerp(vec2 A, float Time, vec2 B) 
{
    return v2Add(v2Scale(A, 1.0f - Time), v2Scale(B, Time));
}

static inline vec3 v3Lerp(vec3 A, float Time, vec3 B) 
{
    return v3Add(v3Scale(A, 1.0f - Time), v3Scale(B, Time));
}

static inline vec4 v4Lerp(vec4 A, float Time, vec4 B) 
{
    return v4Add(v4Scale(A, 1.0f - Time), v4Scale(B, Time));
}

/*
 * SSE stuff
 */

static inline vec4 mat4MakeTransform(vec4 Left, mat4 Right)
{
    
    vec4 Result;
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x00), Right.Columns[0].SSE);
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x55), Right.Columns[1].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xaa), Right.Columns[2].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xff), Right.Columns[3].SSE));
#else
    Result.X = Left.Elements[0] * Right.Columns[0].X;
    Result.Y = Left.Elements[0] * Right.Columns[0].Y;
    Result.Z = Left.Elements[0] * Right.Columns[0].Z;
    Result.W = Left.Elements[0] * Right.Columns[0].W;
    
    Result.X += Left.Elements[1] * Right.Columns[1].X;
    Result.Y += Left.Elements[1] * Right.Columns[1].Y;
    Result.Z += Left.Elements[1] * Right.Columns[1].Z;
    Result.W += Left.Elements[1] * Right.Columns[1].W;
    
    Result.X += Left.Elements[2] * Right.Columns[2].X;
    Result.Y += Left.Elements[2] * Right.Columns[2].Y;
    Result.Z += Left.Elements[2] * Right.Columns[2].Z;
    Result.W += Left.Elements[2] * Right.Columns[2].W;
    
    Result.X += Left.Elements[3] * Right.Columns[3].X;
    Result.Y += Left.Elements[3] * Right.Columns[3].Y;
    Result.Z += Left.Elements[3] * Right.Columns[3].Z;
    Result.W += Left.Elements[3] * Right.Columns[3].W;
#endif
    
    return Result;
}

/*
 * 2x2 Matrices
 */

static inline mat2 mat2Create(void)
{
    mat2 Result = {0};
    return Result;
}

static inline mat2 mat2CreateD(float Diagonal)
{
    
    mat2 Result = {0};
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    
    return Result;
}

static inline mat2 mat2Transpose(mat2 Matrix)
{
    
    mat2 Result = Matrix;
    
    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    
    return Result;
}

static inline mat2 mat2Add(mat2 Left, mat2 Right)
{
    
    mat2 Result;
    
    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
    
    return Result;    
}

static inline mat2 mat2Subtract(mat2 Left, mat2 Right)
{
    
    mat2 Result;
    
    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    
    return Result;
}

static inline vec2 mat2Transform(mat2 Matrix, vec2 Vector)
{
    
    vec2 Result;
    
    Result.X = Vector.Elements[0] * Matrix.Columns[0].X;
    Result.Y = Vector.Elements[0] * Matrix.Columns[0].Y;
    
    Result.X += Vector.Elements[1] * Matrix.Columns[1].X;
    Result.Y += Vector.Elements[1] * Matrix.Columns[1].Y;
    
    return Result;    
}

static inline mat2 mat2Multiply(mat2 Left, mat2 Right)
{
    
    mat2 Result;
    Result.Columns[0] = mat2Transform(Left, Right.Columns[0]);
    Result.Columns[1] = mat2Transform(Left, Right.Columns[1]);
    
    return Result;    
}

static inline mat2 mat2ScaleMatrix(mat2 Matrix, float Scalar)
{
    
    mat2 Result;
    
    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    
    return Result;
}

static inline mat2 mat2Reduce(mat2 Matrix, float Scalar)
{
    
    mat2 Result;
    
    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;
    
    return Result;
}

static inline float mat2Determinate(mat2 Matrix) 
{
    return Matrix.Elements[0][0]*Matrix.Elements[1][1] - Matrix.Elements[0][1]*Matrix.Elements[1][0];
}


static inline mat2 mat2InvDeterminate(mat2 Matrix) 
{
    
    mat2 Result;
    float InvDeterminant = 1.0f / mat2Determinate(Matrix);
    Result.Elements[0][0] = InvDeterminant * +Matrix.Elements[1][1];
    Result.Elements[1][1] = InvDeterminant * +Matrix.Elements[0][0];
    Result.Elements[0][1] = InvDeterminant * -Matrix.Elements[0][1];
    Result.Elements[1][0] = InvDeterminant * -Matrix.Elements[1][0];
    
    return Result;
}

/*
 * 3x3 Matrices
 */

static inline mat3 mat3Create(void)
{
    mat3 Result = {0};
    return Result;
}

static inline mat3 mat3FromMat4(mat4 Transform)
{
    return mat3{
        Transform.Elements[0][0], Transform.Elements[1][0], Transform.Elements[2][0],
        Transform.Elements[0][1], Transform.Elements[1][1], Transform.Elements[2][1],
        Transform.Elements[0][2], Transform.Elements[1][2], Transform.Elements[2][2]
    };
}

static inline mat3 mat3CreateD(float Diagonal)
{
    
    mat3 Result = {0};
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;
    
    return Result;
}

static inline mat3 mat3Transpose(mat3 Matrix)
{
    
    mat3 Result = Matrix;
    
    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[0][2] = Matrix.Elements[2][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    Result.Elements[1][2] = Matrix.Elements[2][1];
    Result.Elements[2][1] = Matrix.Elements[1][2];
    Result.Elements[2][0] = Matrix.Elements[0][2];
    
    return Result;
}

static inline mat3 mat3Add(mat3 Left, mat3 Right)
{
    
    mat3 Result;
    
    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] + Right.Elements[0][2];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] + Right.Elements[1][2];
    Result.Elements[2][0] = Left.Elements[2][0] + Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] + Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] + Right.Elements[2][2];
    
    return Result;    
}

static inline mat3 mat3Subtract(mat3 Left, mat3 Right)
{
    
    mat3 Result;
    
    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] - Right.Elements[0][2];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] - Right.Elements[1][2];
    Result.Elements[2][0] = Left.Elements[2][0] - Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] - Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] - Right.Elements[2][2];
    
    return Result;
}

static inline vec3 mat3Transform(mat3 Matrix, vec3 Vector)
{
    
    vec3 Result;
    
    Result.X = Vector.Elements[0] * Matrix.Columns[0].X;
    Result.Y = Vector.Elements[0] * Matrix.Columns[0].Y;
    Result.Z = Vector.Elements[0] * Matrix.Columns[0].Z;
    
    Result.X += Vector.Elements[1] * Matrix.Columns[1].X;
    Result.Y += Vector.Elements[1] * Matrix.Columns[1].Y;
    Result.Z += Vector.Elements[1] * Matrix.Columns[1].Z;
    
    Result.X += Vector.Elements[2] * Matrix.Columns[2].X;
    Result.Y += Vector.Elements[2] * Matrix.Columns[2].Y;
    Result.Z += Vector.Elements[2] * Matrix.Columns[2].Z;
    
    return Result;    
}

static inline mat3 mat3Multiply(mat3 Left, mat3 Right)
{
    
    mat3 Result;
    Result.Columns[0] = mat3Transform(Left, Right.Columns[0]);
    Result.Columns[1] = mat3Transform(Left, Right.Columns[1]);
    Result.Columns[2] = mat3Transform(Left, Right.Columns[2]);
    
    return Result;    
}

static inline mat3 mat3ScaleMatrix(mat3 Matrix, float Scalar)
{
    
    mat3 Result;
    
    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] * Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] * Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] * Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] * Scalar;
    
    return Result;            
}

static inline mat3 mat3Reduce(mat3 Matrix, float Scalar)
{
    
    mat3 Result;
    
    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] / Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] / Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] / Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] / Scalar;
    
    return Result;                    
}

static inline float mat3Determinate(mat3 Matrix) 
{
    
    mat3 Cross;
    Cross.Columns[0] = v3Cross(Matrix.Columns[1], Matrix.Columns[2]);
    Cross.Columns[1] = v3Cross(Matrix.Columns[2], Matrix.Columns[0]);
    Cross.Columns[2] = v3Cross(Matrix.Columns[0], Matrix.Columns[1]);
    
    return v3Dot(Cross.Columns[2], Matrix.Columns[2]);
}

static inline mat3 mat3Inverse(mat3 Matrix) 
{
    
    mat3 Cross;
    Cross.Columns[0] = v3Cross(Matrix.Columns[1], Matrix.Columns[2]);
    Cross.Columns[1] = v3Cross(Matrix.Columns[2], Matrix.Columns[0]);
    Cross.Columns[2] = v3Cross(Matrix.Columns[0], Matrix.Columns[1]);
    
    float InvDeterminant = 1.0f / v3Dot(Cross.Columns[2], Matrix.Columns[2]);
    
    mat3 Result;
    Result.Columns[0] = v3Scale(Cross.Columns[0], InvDeterminant);
    Result.Columns[1] = v3Scale(Cross.Columns[1], InvDeterminant);
    Result.Columns[2] = v3Scale(Cross.Columns[2], InvDeterminant);
    
    return mat3Transpose(Result);
}

/*
 * 4x4 Matrices
 */

#define NULLMATRIX (mat4Create())
static inline mat4 mat4Create(void)
{
    mat4 Result = {0};
    return Result;
}

static inline mat4 mat4Identity(float Diagonal)
{
    
    mat4 Result = {0};
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;
    Result.Elements[3][3] = Diagonal;
    
    return Result;
}

static inline mat4 mat4Transpose(mat4 Matrix)
{
    
    mat4 Result = Matrix;
#ifdef HANDMADE_MATH__USE_SSE
    _MM_TRANSPOSE4_PS(Result.Columns[0].SSE, Result.Columns[1].SSE, Result.Columns[2].SSE, Result.Columns[3].SSE);
#else
    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[0][2] = Matrix.Elements[2][0];
    Result.Elements[0][3] = Matrix.Elements[3][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    Result.Elements[1][2] = Matrix.Elements[2][1];
    Result.Elements[1][3] = Matrix.Elements[3][1];
    Result.Elements[2][1] = Matrix.Elements[1][2];
    Result.Elements[2][0] = Matrix.Elements[0][2];
    Result.Elements[2][3] = Matrix.Elements[3][2];
    Result.Elements[3][1] = Matrix.Elements[1][3];
    Result.Elements[3][2] = Matrix.Elements[2][3];
    Result.Elements[3][0] = Matrix.Elements[0][3];
#endif
    
    return Result;
}

static inline mat4 mat4Add(mat4 Left, mat4 Right)
{
    
    mat4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0].SSE = _mm_add_ps(Left.Columns[0].SSE, Right.Columns[0].SSE);
    Result.Columns[1].SSE = _mm_add_ps(Left.Columns[1].SSE, Right.Columns[1].SSE);
    Result.Columns[2].SSE = _mm_add_ps(Left.Columns[2].SSE, Right.Columns[2].SSE);
    Result.Columns[3].SSE = _mm_add_ps(Left.Columns[3].SSE, Right.Columns[3].SSE);
#else
    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] + Right.Elements[0][2];
    Result.Elements[0][3] = Left.Elements[0][3] + Right.Elements[0][3];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] + Right.Elements[1][2];
    Result.Elements[1][3] = Left.Elements[1][3] + Right.Elements[1][3];
    Result.Elements[2][0] = Left.Elements[2][0] + Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] + Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] + Right.Elements[2][2];
    Result.Elements[2][3] = Left.Elements[2][3] + Right.Elements[2][3];
    Result.Elements[3][0] = Left.Elements[3][0] + Right.Elements[3][0];
    Result.Elements[3][1] = Left.Elements[3][1] + Right.Elements[3][1];
    Result.Elements[3][2] = Left.Elements[3][2] + Right.Elements[3][2];
    Result.Elements[3][3] = Left.Elements[3][3] + Right.Elements[3][3];
#endif
    
    return Result;
}

static inline mat4 mat4Subtract(mat4 Left, mat4 Right)
{
    
    mat4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0].SSE = _mm_sub_ps(Left.Columns[0].SSE, Right.Columns[0].SSE);
    Result.Columns[1].SSE = _mm_sub_ps(Left.Columns[1].SSE, Right.Columns[1].SSE);
    Result.Columns[2].SSE = _mm_sub_ps(Left.Columns[2].SSE, Right.Columns[2].SSE);
    Result.Columns[3].SSE = _mm_sub_ps(Left.Columns[3].SSE, Right.Columns[3].SSE);
#else
    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] - Right.Elements[0][2];
    Result.Elements[0][3] = Left.Elements[0][3] - Right.Elements[0][3];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] - Right.Elements[1][2];
    Result.Elements[1][3] = Left.Elements[1][3] - Right.Elements[1][3];
    Result.Elements[2][0] = Left.Elements[2][0] - Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] - Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] - Right.Elements[2][2];
    Result.Elements[2][3] = Left.Elements[2][3] - Right.Elements[2][3];
    Result.Elements[3][0] = Left.Elements[3][0] - Right.Elements[3][0];
    Result.Elements[3][1] = Left.Elements[3][1] - Right.Elements[3][1];
    Result.Elements[3][2] = Left.Elements[3][2] - Right.Elements[3][2];
    Result.Elements[3][3] = Left.Elements[3][3] - Right.Elements[3][3];
#endif
    
    return Result;
}

static inline mat4 mat4Multiply(mat4 Left, mat4 Right)
{
    
    mat4 Result;
    Result.Columns[0] = mat4MakeTransform(Right.Columns[0], Left);
    Result.Columns[1] = mat4MakeTransform(Right.Columns[1], Left);
    Result.Columns[2] = mat4MakeTransform(Right.Columns[2], Left);
    Result.Columns[3] = mat4MakeTransform(Right.Columns[3], Left);
    
    return Result;
}

static inline mat4 mat4ScaleMatrix(mat4 Matrix, float Scalar)
{
    
    mat4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_mul_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_mul_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_mul_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_mul_ps(Matrix.Columns[3].SSE, SSEScalar);
#else
    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] * Scalar;
    Result.Elements[0][3] = Matrix.Elements[0][3] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] * Scalar;
    Result.Elements[1][3] = Matrix.Elements[1][3] * Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] * Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] * Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] * Scalar;
    Result.Elements[2][3] = Matrix.Elements[2][3] * Scalar;
    Result.Elements[3][0] = Matrix.Elements[3][0] * Scalar;
    Result.Elements[3][1] = Matrix.Elements[3][1] * Scalar;
    Result.Elements[3][2] = Matrix.Elements[3][2] * Scalar;
    Result.Elements[3][3] = Matrix.Elements[3][3] * Scalar;
#endif
    
    return Result;
}

static inline vec4 mat4Transform(mat4 Matrix, vec4 Vector)
{
    return mat4MakeTransform(Vector, Matrix);
}

static inline mat4 mat4Reduce(mat4 Matrix, float Scalar)
{
    
    mat4 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_div_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_div_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_div_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_div_ps(Matrix.Columns[3].SSE, SSEScalar);
#else
    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] / Scalar;
    Result.Elements[0][3] = Matrix.Elements[0][3] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] / Scalar;
    Result.Elements[1][3] = Matrix.Elements[1][3] / Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] / Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] / Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] / Scalar;
    Result.Elements[2][3] = Matrix.Elements[2][3] / Scalar;
    Result.Elements[3][0] = Matrix.Elements[3][0] / Scalar;
    Result.Elements[3][1] = Matrix.Elements[3][1] / Scalar;
    Result.Elements[3][2] = Matrix.Elements[3][2] / Scalar;
    Result.Elements[3][3] = Matrix.Elements[3][3] / Scalar;
#endif
    
    return Result;
}

static inline float mat4Determinate(mat4 Matrix) 
{
    
    vec3 C01 = v3Cross(Matrix.Columns[0].XYZ, Matrix.Columns[1].XYZ);
    vec3 C23 = v3Cross(Matrix.Columns[2].XYZ, Matrix.Columns[3].XYZ);
    vec3 B10 = v3Subtract(v3Scale(Matrix.Columns[0].XYZ, Matrix.Columns[1].W), v3Scale(Matrix.Columns[1].XYZ, Matrix.Columns[0].W));
    vec3 B32 = v3Subtract(v3Scale(Matrix.Columns[2].XYZ, Matrix.Columns[3].W), v3Scale(Matrix.Columns[3].XYZ, Matrix.Columns[2].W));
    
    return v3Dot(C01, B32) + v3Dot(C23, B10);
}

// Returns a general-purpose inverse of an mat4. Note that special-purpose inverses of many transformations
// are available and will be more efficient.
static inline mat4 mat4Inverse(mat4 Matrix) 
{
    
    vec3 C01 = v3Cross(Matrix.Columns[0].XYZ, Matrix.Columns[1].XYZ);
    vec3 C23 = v3Cross(Matrix.Columns[2].XYZ, Matrix.Columns[3].XYZ);
    vec3 B10 = v3Subtract(v3Scale(Matrix.Columns[0].XYZ, Matrix.Columns[1].W), v3Scale(Matrix.Columns[1].XYZ, Matrix.Columns[0].W));
    vec3 B32 = v3Subtract(v3Scale(Matrix.Columns[2].XYZ, Matrix.Columns[3].W), v3Scale(Matrix.Columns[3].XYZ, Matrix.Columns[2].W));
    
    float InvDeterminant = 1.0f / (v3Dot(C01, B32) + v3Dot(C23, B10));
    C01 = v3Scale(C01, InvDeterminant);
    C23 = v3Scale(C23, InvDeterminant);
    B10 = v3Scale(B10, InvDeterminant);
    B32 = v3Scale(B32, InvDeterminant);
    
    mat4 Result;
    Result.Columns[0] = v3Expand(v3Add(v3Cross(Matrix.Columns[1].XYZ, B32), v3Scale(C23, Matrix.Columns[1].W)), -v3Dot(Matrix.Columns[1].XYZ, C23));
    Result.Columns[1] = v3Expand(v3Subtract(v3Cross(B32, Matrix.Columns[0].XYZ), v3Scale(C23, Matrix.Columns[0].W)), +v3Dot(Matrix.Columns[0].XYZ, C23));
    Result.Columns[2] = v3Expand(v3Add(v3Cross(Matrix.Columns[3].XYZ, B10), v3Scale(C01, Matrix.Columns[3].W)), -v3Dot(Matrix.Columns[3].XYZ, C01));
    Result.Columns[3] = v3Expand(v3Subtract(v3Cross(B10, Matrix.Columns[2].XYZ), v3Scale(C01, Matrix.Columns[2].W)), +v3Dot(Matrix.Columns[2].XYZ, C01));
    
    return mat4Transpose(Result);
}

/*
 * Common graphics transformations
 */

// Produces a right-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4RHGLOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    
    mat4 Result = {0};
    
    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 2.0f / (Near - Far);
    Result.Elements[3][3] = 1.0f;
    
    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Near + Far) / (Near - Far);
    
    return Result;
}

// Produces a right-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4RHDXOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    
    mat4 Result = {0};
    
    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 1.0f / (Near - Far);
    Result.Elements[3][3] = 1.0f;
    
    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Near) / (Near - Far);
    
    return Result;
}

// Produces a left-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4LHGLOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    
    mat4 Result = mat4RHGLOrtho(Left, Right, Bottom, Top, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    
    return Result;
}

// Produces a left-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4LHDXOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    
    mat4 Result = mat4RHDXOrtho(Left, Right, Bottom, Top, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    
    return Result;
}

// Returns an inverse for the given orthographic projection matrix. Works for all orthographic
// projection matrices, regardless of handedness or NDC convention.
static inline mat4 mat4OrthoInverse(mat4 OrthoMatrix)
{
    
    mat4 Result = {0};
    Result.Elements[0][0] = 1.0f / OrthoMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / OrthoMatrix.Elements[1][1];
    Result.Elements[2][2] = 1.0f / OrthoMatrix.Elements[2][2];
    Result.Elements[3][3] = 1.0f;
    
    Result.Elements[3][0] = -OrthoMatrix.Elements[3][0] * Result.Elements[0][0];
    Result.Elements[3][1] = -OrthoMatrix.Elements[3][1] * Result.Elements[1][1];
    Result.Elements[3][2] = -OrthoMatrix.Elements[3][2] * Result.Elements[2][2];
    
    return Result;
}

static inline mat4 mat4RHGLMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{
    
    mat4 Result = {0};
    
    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
    
    float Cotangent = 1.0f / TanF(FOV / 2.0f);
    Result.Elements[0][0] = Cotangent / AspectRatio;
    Result.Elements[1][1] = Cotangent;
    Result.Elements[2][3] = -1.0f;
    
    Result.Elements[2][2] = (Near + Far) / (Near - Far);
    Result.Elements[3][2] = (2.0f * Near * Far) / (Near - Far);
    
    return Result;
}

static inline mat4 mat4RLDXMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{
    
    mat4 Result = {0};
    
    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
    
    float Cotangent = 1.0f / TanF(FOV / 2.0f);
    Result.Elements[0][0] = Cotangent / AspectRatio;
    Result.Elements[1][1] = Cotangent;
    Result.Elements[2][3] = -1.0f;
    
    Result.Elements[2][2] = (Far) / (Near - Far);
    Result.Elements[3][2] = (Near * Far) / (Near - Far);
    
    return Result;
}

static inline mat4 mat4LHGLMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{ 
    
    mat4 Result = mat4RHGLMakePerspective(FOV, AspectRatio, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    Result.Elements[2][3] = -Result.Elements[2][3];
    
    return Result;
}

static inline mat4 mat4LHDXMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{ 
    
    mat4 Result = mat4RLDXMakePerspective(FOV, AspectRatio, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    Result.Elements[2][3] = -Result.Elements[2][3];
    
    return Result;
}

static inline mat4 mat4RHInversePerp(mat4 PerspectiveMatrix)
{
    
    mat4 Result = {0};
    Result.Elements[0][0] = 1.0f / PerspectiveMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / PerspectiveMatrix.Elements[1][1];
    Result.Elements[2][2] = 0.0f;
    
    Result.Elements[2][3] = 1.0f / PerspectiveMatrix.Elements[3][2];
    Result.Elements[3][3] = PerspectiveMatrix.Elements[2][2] * Result.Elements[2][3];
    Result.Elements[3][2] = PerspectiveMatrix.Elements[2][3];
    
    return Result;
}

static inline mat4 mat4LHInversePerp(mat4 PerspectiveMatrix)
{
    
    mat4 Result = {0};
    Result.Elements[0][0] = 1.0f / PerspectiveMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / PerspectiveMatrix.Elements[1][1];
    Result.Elements[2][2] = 0.0f;
    
    Result.Elements[2][3] = 1.0f / PerspectiveMatrix.Elements[3][2];
    Result.Elements[3][3] = PerspectiveMatrix.Elements[2][2] * -Result.Elements[2][3];
    Result.Elements[3][2] = PerspectiveMatrix.Elements[2][3];
    
    return Result;
}

static inline mat4 mat4Translate(vec3 Translation)
{
    
    mat4 Result = mat4Identity(1.0f);
    Result.Elements[3][0] = Translation.X;
    Result.Elements[3][1] = Translation.Y;
    Result.Elements[3][2] = Translation.Z;
    
    return Result;
}

static inline mat4 mat4Translation(mat4 A, vec3 Translation)
{
    return(mat4Multiply(A, mat4Translate(Translation)));
}

static inline mat4 mat4InvTranslate(mat4 TranslationMatrix)
{
    
    mat4 Result = TranslationMatrix;
    Result.Elements[3][0] = -Result.Elements[3][0];
    Result.Elements[3][1] = -Result.Elements[3][1];
    Result.Elements[3][2] = -Result.Elements[3][2];
    
    return Result;
}

static inline mat4 mat4RHRotate(float Angle, vec3 Axis)
{
    
    mat4 Result = mat4Identity(1.0f);
    
    Axis = v3Normalize(Axis);
    
    float SinTheta = SinF(Angle);
    float CosTheta = CosF(Angle);
    float CosValue = 1.0f - CosTheta;
    
    Result.Elements[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.Elements[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.Elements[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);
    
    Result.Elements[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.Elements[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.Elements[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);
    
    Result.Elements[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.Elements[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.Elements[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;
    
    return Result;
}

static inline mat4 mat4LHRotate(float Angle, vec3 Axis)
{
    /* NOTE(lcf): Matrix will be inverse/transpose of RH. */
    return mat4RHRotate(-Angle, Axis);
}

static inline mat4 mat4InverseRotate(mat4 RotationMatrix)
{
    return mat4Transpose(RotationMatrix);
}

static inline mat4 mat4MakeScale(vec3 Scale)
{
    
    mat4 Result = mat4Identity(1.0f);
    Result.Elements[0][0] = Scale.X;
    Result.Elements[1][1] = Scale.Y;
    Result.Elements[2][2] = Scale.Z;
    
    return Result;
}

static inline mat4 mat4Scale(mat4 A, vec3 Scale)
{
    return(mat4Multiply(A, mat4MakeScale(Scale)));
}

static inline mat4 mat4InvScale(mat4 ScaleMatrix) 
{
    
    mat4 Result = ScaleMatrix;
    Result.Elements[0][0] = 1.0f / Result.Elements[0][0];
    Result.Elements[1][1] = 1.0f / Result.Elements[1][1];
    Result.Elements[2][2] = 1.0f / Result.Elements[2][2];
    
    return Result;
}

static inline mat4 _LookAt(vec3 F,  vec3 S, vec3 U,  vec3 Eye)
{
    mat4 Result;
    
    Result.Elements[0][0] = S.X;
    Result.Elements[0][1] = U.X;
    Result.Elements[0][2] = -F.X;
    Result.Elements[0][3] = 0.0f;
    
    Result.Elements[1][0] = S.Y;
    Result.Elements[1][1] = U.Y;
    Result.Elements[1][2] = -F.Y;
    Result.Elements[1][3] = 0.0f;
    
    Result.Elements[2][0] = S.Z;
    Result.Elements[2][1] = U.Z;
    Result.Elements[2][2] = -F.Z;
    Result.Elements[2][3] = 0.0f;
    
    Result.Elements[3][0] = -v3Dot(S, Eye);
    Result.Elements[3][1] = -v3Dot(U, Eye);
    Result.Elements[3][2] = v3Dot(F, Eye);
    Result.Elements[3][3] = 1.0f;
    
    return Result;
}


static inline mat4 LookAt_RH(vec3 Eye, vec3 Center, vec3 Up)
{
    
    vec3 F = v3Normalize(v3Subtract(Center, Eye));
    vec3 S = v3Normalize(v3Cross(F, Up));
    vec3 U = v3Cross(S, F);
    
    return _LookAt(F, S, U, Eye);
}

static inline mat4 LookAt_LH(vec3 Eye, vec3 Center, vec3 Up)
{
    
    vec3 F = v3Normalize(v3Subtract(Eye, Center));
    vec3 S = v3Normalize(v3Cross(F, Up));
    vec3 U = v3Cross(S, F);
    
    return _LookAt(F, S, U, Eye);
}

static inline mat4 InvLookAt(mat4 Matrix)
{
    mat4 Result;
    
    mat3 Rotation = {0};
    Rotation.Columns[0] = Matrix.Columns[0].XYZ;
    Rotation.Columns[1] = Matrix.Columns[1].XYZ;
    Rotation.Columns[2] = Matrix.Columns[2].XYZ;
    Rotation = mat3Transpose(Rotation);
    
    Result.Columns[0] = v3Expand(Rotation.Columns[0], 0.0f);
    Result.Columns[1] = v3Expand(Rotation.Columns[1], 0.0f);
    Result.Columns[2] = v3Expand(Rotation.Columns[2], 0.0f);
    Result.Columns[3] = v4Scale(Matrix.Columns[3], -1.0f);
    Result.Elements[3][0] = -1.0f * Matrix.Elements[3][0] /
    (Rotation.Elements[0][0] + Rotation.Elements[0][1] + Rotation.Elements[0][2]);
    Result.Elements[3][1] = -1.0f * Matrix.Elements[3][1] /
    (Rotation.Elements[1][0] + Rotation.Elements[1][1] + Rotation.Elements[1][2]);
    Result.Elements[3][2] = -1.0f * Matrix.Elements[3][2] /
    (Rotation.Elements[2][0] + Rotation.Elements[2][1] + Rotation.Elements[2][2]);
    Result.Elements[3][3] = 1.0f;
    
    return Result;
}

/*
 * Quaternion operations
 */

static inline quat quatCreate(float X, float Y, float Z, float W)
{
    
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif
    
    return Result;
}

static inline quat quatCreateV4(vec4 Vector)
{
    
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = Vector.SSE;
#else
    Result.X = Vector.X;
    Result.Y = Vector.Y;
    Result.Z = Vector.Z;
    Result.W = Vector.W;
#endif
    
    return Result;
}

static inline quat quatAdd(quat Left, quat Right)
{
    
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#else
    
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif
    
    return Result;
}

static inline quat quatSubtract(quat Left, quat Right)
{
    
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif
    
    return Result;
}

static inline quat MulQ(quat Left, quat Right)
{
    
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
    __m128 SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(0, 1, 2, 3));
    __m128 SSEResultThree = _mm_mul_ps(SSEResultTwo, SSEResultOne);
    
    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(1, 1, 1, 1)) , _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(1, 0, 3, 2));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
    
    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
    
    SSEResultOne = _mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(3, 3, 3, 3));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(3, 2, 1, 0));
    Result.SSE = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
#else
    Result.X =  Right.Elements[3] * +Left.Elements[0];
    Result.Y =  Right.Elements[2] * -Left.Elements[0];
    Result.Z =  Right.Elements[1] * +Left.Elements[0];
    Result.W =  Right.Elements[0] * -Left.Elements[0];
    
    Result.X += Right.Elements[2] * +Left.Elements[1];
    Result.Y += Right.Elements[3] * +Left.Elements[1];
    Result.Z += Right.Elements[0] * -Left.Elements[1];
    Result.W += Right.Elements[1] * -Left.Elements[1];
    
    Result.X += Right.Elements[1] * -Left.Elements[2];
    Result.Y += Right.Elements[0] * +Left.Elements[2];
    Result.Z += Right.Elements[3] * +Left.Elements[2];
    Result.W += Right.Elements[2] * -Left.Elements[2];
    
    Result.X += Right.Elements[0] * +Left.Elements[3];
    Result.Y += Right.Elements[1] * +Left.Elements[3];
    Result.Z += Right.Elements[2] * +Left.Elements[3];
    Result.W += Right.Elements[3] * +Left.Elements[3];
#endif
    
    return Result;
}

static inline quat quatScale(quat Left, float Multiplicative)
{
    
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Multiplicative);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X * Multiplicative;
    Result.Y = Left.Y * Multiplicative;
    Result.Z = Left.Z * Multiplicative;
    Result.W = Left.W * Multiplicative;
#endif
    
    return Result;
}

static inline quat quatReduce(quat Left, float Divnd)
{
    
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Divnd);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X / Divnd;
    Result.Y = Left.Y / Divnd;
    Result.Z = Left.Z / Divnd;
    Result.W = Left.W / Divnd;
#endif
    
    return Result;
}

static inline float DotQ(quat Left, quat Right)
{
    
    float Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = ((Left.X * Right.X) + (Left.Z * Right.Z)) + ((Left.Y * Right.Y) + (Left.W * Right.W));
#endif
    
    return Result;
}

static inline quat InvQ(quat Left)
{
    
    quat Result;
    Result.X = -Left.X;
    Result.Y = -Left.Y;
    Result.Z = -Left.Z;
    Result.W = Left.W;
    
    return quatReduce(Result, (DotQ(Left, Left)));
}

static inline quat NormQ(quat Quat)
{
    
    /* NOTE(lcf): Take advantage of SSE implementation in v4Normalize */
    vec4 Vec = {Quat.X, Quat.Y, Quat.Z, Quat.W};
    Vec = v4Normalize(Vec);
    quat Result = {Vec.X, Vec.Y, Vec.Z, Vec.W};
    
    return Result;
}

static inline quat _MixQ(quat Left, float MixLeft, quat Right, float MixRight) {
    quat Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 ScalarLeft = _mm_set1_ps(MixLeft);
    __m128 ScalarRight = _mm_set1_ps(MixRight);
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, ScalarLeft);
    __m128 SSEResultTwo = _mm_mul_ps(Right.SSE, ScalarRight);
    Result.SSE = _mm_add_ps(SSEResultOne, SSEResultTwo);
#else
    Result.X = Left.X*MixLeft + Right.X*MixRight;
    Result.Y = Left.Y*MixLeft + Right.Y*MixRight;
    Result.Z = Left.Z*MixLeft + Right.Z*MixRight;
    Result.W = Left.W*MixLeft + Right.W*MixRight;
#endif
    
    return Result;
}

static inline quat NLerp(quat Left, float Time, quat Right)
{
    
    quat Result = _MixQ(Left, 1.0f-Time, Right, Time);
    Result = NormQ(Result);
    
    return Result;
}

static inline quat SLerp(quat Left, float Time, quat Right)
{
    
    quat Result;
    
    float Cos_Theta = DotQ(Left, Right);
    
    if (Cos_Theta < 0.0f) { /* NOTE(lcf): Take shortest path on Hyper-sphere */
        Cos_Theta = -Cos_Theta;
        Right = quatCreate(-Right.X, -Right.Y, -Right.Z, -Right.W);
    }
    
    /* NOTE(lcf): Use Normalized Linear interpolation when vectors are roughly not L.I. */
    if (Cos_Theta > 0.9995f) {
        Result = NLerp(Left, Time, Right);
    } else {
        float Angle = ACosF(Cos_Theta);
        float MixLeft = SinF((1.0f - Time) * Angle);
        float MixRight = SinF(Time * Angle);
        
        Result = _MixQ(Left, MixLeft, Right, MixRight);
        Result = NormQ(Result);
    }
    
    return Result;
}

static inline mat4 quatCreateToM4(quat Left)
{
    
    mat4 Result;
    
    quat NormalizedQ = NormQ(Left);
    
    float XX, YY, ZZ,
    XY, XZ, YZ,
    WX, WY, WZ;
    
    XX = NormalizedQ.X * NormalizedQ.X;
    YY = NormalizedQ.Y * NormalizedQ.Y;
    ZZ = NormalizedQ.Z * NormalizedQ.Z;
    XY = NormalizedQ.X * NormalizedQ.Y;
    XZ = NormalizedQ.X * NormalizedQ.Z;
    YZ = NormalizedQ.Y * NormalizedQ.Z;
    WX = NormalizedQ.W * NormalizedQ.X;
    WY = NormalizedQ.W * NormalizedQ.Y;
    WZ = NormalizedQ.W * NormalizedQ.Z;
    
    Result.Elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.Elements[0][1] = 2.0f * (XY + WZ);
    Result.Elements[0][2] = 2.0f * (XZ - WY);
    Result.Elements[0][3] = 0.0f;
    
    Result.Elements[1][0] = 2.0f * (XY - WZ);
    Result.Elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.Elements[1][2] = 2.0f * (YZ + WX);
    Result.Elements[1][3] = 0.0f;
    
    Result.Elements[2][0] = 2.0f * (XZ + WY);
    Result.Elements[2][1] = 2.0f * (YZ - WX);
    Result.Elements[2][2] = 1.0f - 2.0f * (XX + YY);
    Result.Elements[2][3] = 0.0f;
    
    Result.Elements[3][0] = 0.0f;
    Result.Elements[3][1] = 0.0f;
    Result.Elements[3][2] = 0.0f;
    Result.Elements[3][3] = 1.0f;
    
    return Result;
}

// This method taken from Mike Day at Insomniac Games.
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
//
// Note that as mentioned at the top of the paper, the paper assumes the matrix
// would be *post*-multiplied to a vector to rotate it, meaning the matrix is
// the transpose of what we're dealing with. But, because our matrices are
// stored in column-major order, the indices *appear* to match the paper.
//
// For example, m12 in the paper is row 1, column 2. We need to transpose it to
// row 2, column 1. But, because the column comes first when referencing
// elements, it looks like M.Elements[1][2].
//
// Don't be confused! Or if you must be confused, at least trust this
// comment. :)
static inline quat mat4CreateToQ_RH(mat4 M)
{
    float T;
    quat Q;
    
    if (M.Elements[2][2] < 0.0f) {
        if (M.Elements[0][0] > M.Elements[1][1]) {
            
            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                           T,
                           M.Elements[0][1] + M.Elements[1][0],
                           M.Elements[2][0] + M.Elements[0][2],
                           M.Elements[1][2] - M.Elements[2][1]
                           );
        } else {
            
            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                           M.Elements[0][1] + M.Elements[1][0],
                           T,
                           M.Elements[1][2] + M.Elements[2][1],
                           M.Elements[2][0] - M.Elements[0][2]
                           );
        }
    } else {
        if (M.Elements[0][0] < -M.Elements[1][1]) {
            
            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                           M.Elements[2][0] + M.Elements[0][2],
                           M.Elements[1][2] + M.Elements[2][1],
                           T,
                           M.Elements[0][1] - M.Elements[1][0]
                           );
        } else {
            
            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                           M.Elements[1][2] - M.Elements[2][1],
                           M.Elements[2][0] - M.Elements[0][2],
                           M.Elements[0][1] - M.Elements[1][0],
                           T
                           );
        }
    }
    
    Q = quatScale(Q, 0.5f / SqrtF(T));
    
    return Q;
}

static inline quat mat4CreateToQ_LH(mat4 M)
{
    float T;
    quat Q;
    
    if (M.Elements[2][2] < 0.0f) {
        if (M.Elements[0][0] > M.Elements[1][1]) {
            
            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                           T,
                           M.Elements[0][1] + M.Elements[1][0],
                           M.Elements[2][0] + M.Elements[0][2],
                           M.Elements[2][1] - M.Elements[1][2]
                           );
        } else {
            
            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                           M.Elements[0][1] + M.Elements[1][0],
                           T,
                           M.Elements[1][2] + M.Elements[2][1],
                           M.Elements[0][2] - M.Elements[2][0]
                           );
        }
    } else {
        if (M.Elements[0][0] < -M.Elements[1][1]) {
            
            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                           M.Elements[2][0] + M.Elements[0][2],
                           M.Elements[1][2] + M.Elements[2][1],
                           T,
                           M.Elements[1][0] - M.Elements[0][1]
                           );
        } else {
            
            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                           M.Elements[2][1] - M.Elements[1][2],
                           M.Elements[0][2] - M.Elements[2][0],
                           M.Elements[1][0] - M.Elements[0][2],
                           T
                           );
        }
    }
    
    Q = quatScale(Q, 0.5f / SqrtF(T));
    
    return Q;
}


static inline quat quatCreateFromAxisAngle_RH(vec3 Axis, float AngleOfRotation)
{
    
    quat Result;
    
    vec3 AxisNormalized = v3Normalize(Axis);
    float SineOfRotation = SinF(AngleOfRotation / 2.0f);
    
    Result.XYZ = v3Scale(AxisNormalized, SineOfRotation);
    Result.W = CosF(AngleOfRotation / 2.0f);
    
    return Result;
}

static inline quat quatCreateFromAxisAngle_LH(vec3 Axis, float AngleOfRotation)
{
    
    return quatCreateFromAxisAngle_RH(Axis, -AngleOfRotation);
}

static inline float
r32Squared(float Value)
{
    return(Value * Value);
}

#ifdef __cplusplus

static inline float Length(vec2 A)
{
    return v2Length(A);
}

static inline float Length(vec3 A)
{
    return v3Length(A);
}

static inline float Length(vec4 A)
{
    return v4Length(A);
}

static inline float LengthSquared(vec2 A)
{
    return v2LengthSquared(A);
}

static inline float LengthSquared(vec3 A)
{
    return v3LengthSquared(A);
}

static inline float LengthSquared(vec4 A)
{
    return v4LengthSquared(A);
}

static inline vec2 Normalized(vec2 A)
{
    return v2Normalize(A);
}

static inline vec3 Normalized(vec3 A)
{
    return v3Normalize(A);
}

static inline vec4 Normalized(vec4 A)
{
    return v4Normalize(A);
}

static inline quat Normalized(quat A)
{
    return NormQ(A);
}

static inline float Dot(vec2 Left, vec2 VecTwo)
{
    return v2Dot(Left, VecTwo);
}

static inline float Dot(vec3 Left, vec3 VecTwo)
{
    return v3Dot(Left, VecTwo);
}

static inline float Dot(vec4 Left, vec4 VecTwo)
{
    return v4Dot(Left, VecTwo);
}

static inline vec2 Lerp(vec2 Left, float Time, vec2 Right) 
{
    return v2Lerp(Left, Time, Right);
}

static inline vec3 Lerp(vec3 Left, float Time, vec3 Right) 
{
    return v3Lerp(Left, Time, Right);
}

static inline vec4 Lerp(vec4 Left, float Time, vec4 Right) 
{
    return v4Lerp(Left, Time, Right);
}

static inline mat2 Transpose(mat2 Matrix)
{
    return mat2Transpose(Matrix);
}

static inline mat3 Transpose(mat3 Matrix)
{
    return mat3Transpose(Matrix);
}

static inline mat4 Transpose(mat4 Matrix)
{
    return mat4Transpose(Matrix);
}

static inline float Determinant(mat2 Matrix)
{
    return mat2Determinate(Matrix);
}

static inline float Determinant(mat3 Matrix)
{
    return mat3Determinate(Matrix);
}

static inline float Determinant(mat4 Matrix)
{
    return mat4Determinate(Matrix);
}

static inline mat2 Inverse(mat2 Matrix)
{
    return mat2InvDeterminate(Matrix);
}

static inline mat3 Inverse(mat3 Matrix)
{
    return mat3Inverse(Matrix);
}

static inline mat4 Inverse(mat4 Matrix)
{
    return mat4Inverse(Matrix);
}

static inline float Dot(quat QuatOne, quat QuatTwo)
{
    return DotQ(QuatOne, QuatTwo);
}

static inline vec2 Add(vec2 Left, vec2 Right)
{
    return v2Add(Left, Right);
}

static inline vec3 Add(vec3 Left, vec3 Right)
{
    return v3Add(Left, Right);
}

static inline vec4 Add(vec4 Left, vec4 Right)
{
    return v4Add(Left, Right);
}

static inline mat2 Add(mat2 Left, mat2 Right)
{
    return mat2Add(Left, Right);
}

static inline mat3 Add(mat3 Left, mat3 Right)
{
    return mat3Add(Left, Right);
}

static inline mat4 Add(mat4 Left, mat4 Right)
{
    return mat4Add(Left, Right);
}

static inline quat Add(quat Left, quat Right)
{
    return quatAdd(Left, Right);
}

static inline vec2 Subtract(vec2 Left, vec2 Right)
{
    return v2Subtract(Left, Right);
}

static inline vec3 Subtract(vec3 Left, vec3 Right)
{
    return v3Subtract(Left, Right);
}

static inline vec4 Subtract(vec4 Left, vec4 Right)
{
    return v4Subtract(Left, Right);
}

static inline mat2 Subtract(mat2 Left, mat2 Right)
{
    return mat2Subtract(Left, Right);
}

static inline mat3 Subtract(mat3 Left, mat3 Right)
{
    return mat3Subtract(Left, Right);
}

static inline mat4 Subtract(mat4 Left, mat4 Right)
{
    return mat4Subtract(Left, Right);
}

static inline quat Subtract(quat Left, quat Right)
{
    return quatSubtract(Left, Right);
}

static inline vec2 Multiply(vec2 Left, vec2 Right)
{
    return v2Multiply(Left, Right);
}

static inline vec2 Multiply(vec2 Left, float Right)
{
    return v2Scale(Left, Right);
}

static inline vec3 Multiply(vec3 Left, vec3 Right)
{
    return v3Multiply(Left, Right);
}

static inline vec3 Multiply(vec3 Left, float Right)
{
    return v3Scale(Left, Right);
}

static inline vec4 Multiply(vec4 Left, vec4 Right)
{
    return v4Multiply(Left, Right);
}

static inline vec4 Multiply(vec4 Left, float Right)
{
    return v4Scale(Left, Right);
}

static inline mat2 Multiply(mat2 Left, mat2 Right)
{
    return mat2Multiply(Left, Right);
}

static inline mat3 Multiply(mat3 Left, mat3 Right)
{
    return mat3Multiply(Left, Right);
}

static inline mat4 Multiply(mat4 Left, mat4 Right)
{
    return mat4Multiply(Left, Right);
}

static inline mat2 Multiply(mat2 Left, float Right)
{
    return mat2ScaleMatrix(Left, Right);
}

static inline mat3 Multiply(mat3 Left, float Right)
{
    return mat3ScaleMatrix(Left, Right);
}

static inline mat4 Multiply(mat4 Left, float Right)
{
    return mat4ScaleMatrix(Left, Right);
}

static inline vec2 Multiply(mat2 Matrix, vec2 Vector)
{
    return mat2Transform(Matrix, Vector);
}

static inline vec3 Multiply(mat3 Matrix, vec3 Vector)
{
    return mat3Transform(Matrix, Vector);
}

static inline vec4 Multiply(mat4 Matrix, vec4 Vector)
{
    return mat4Transform(Matrix, Vector);
}

static inline quat Multiply(quat Left, quat Right)
{
    return MulQ(Left, Right);
}

static inline quat Multiply(quat Left, float Right)
{
    return quatScale(Left, Right);
}

static inline vec2 Divide(vec2 Left, vec2 Right)
{
    return v2Divide(Left, Right);
}

static inline vec2 Divide(vec2 Left, float Right)
{
    return v2Reduce(Left, Right);
}

static inline vec3 Divide(vec3 Left, vec3 Right)
{
    return v3Divide(Left, Right);
}

static inline vec3 Divide(vec3 Left, float Right)
{
    return v3Reduce(Left, Right);
}

static inline vec4 Divide(vec4 Left, vec4 Right)
{
    return v4Divide(Left, Right);
}

static inline vec4 Divide(vec4 Left, float Right)
{
    return v4Reduce(Left, Right);
}

static inline mat2 Divide(mat2 Left, float Right)
{
    return mat2Reduce(Left, Right);
}

static inline mat3 Divide(mat3 Left, float Right)
{
    return mat3Reduce(Left, Right);
}

static inline mat4 Divide(mat4 Left, float Right)
{
    return mat4Reduce(Left, Right);
}

static inline quat Divide(quat Left, float Right)
{
    return quatReduce(Left, Right);
}

static inline Bool IsEqual(vec2 Left, vec2 Right)
{
    return v2Equal(Left, Right);
}

static inline Bool IsEqual(vec3 Left, vec3 Right)
{
    return v3Equal(Left, Right);
}

static inline Bool IsEqual(vec4 Left, vec4 Right)
{
    return v4Equal(Left, Right);
}

static inline vec2 operator+(vec2 Left, vec2 Right)
{
    return v2Add(Left, Right);
}

static inline vec3 operator+(vec3 Left, vec3 Right)
{
    return v3Add(Left, Right);
}

static inline vec4 operator+(vec4 Left, vec4 Right)
{
    return v4Add(Left, Right);
}

static inline mat2 operator+(mat2 Left, mat2 Right)
{
    return mat2Add(Left, Right);
}

static inline mat3 operator+(mat3 Left, mat3 Right)
{
    return mat3Add(Left, Right);
}

static inline mat4 operator+(mat4 Left, mat4 Right)
{
    return mat4Add(Left, Right);
}

static inline quat operator+(quat Left, quat Right)
{
    return quatAdd(Left, Right);
}

static inline vec2 operator-(vec2 Left, vec2 Right)
{
    return v2Subtract(Left, Right);
}

static inline vec3 operator-(vec3 Left, vec3 Right)
{
    return v3Subtract(Left, Right);
}

static inline vec4 operator-(vec4 Left, vec4 Right)
{
    return v4Subtract(Left, Right);
}

static inline mat2 operator-(mat2 Left, mat2 Right)
{
    return mat2Subtract(Left, Right);
}

static inline mat3 operator-(mat3 Left, mat3 Right)
{
    return mat3Subtract(Left, Right);
}

static inline mat4 operator-(mat4 Left, mat4 Right)
{
    return mat4Subtract(Left, Right);
}

static inline quat operator-(quat Left, quat Right)
{
    return quatSubtract(Left, Right);
}

static inline vec2 operator*(vec2 Left, vec2 Right)
{
    return v2Multiply(Left, Right);
}

static inline vec3 operator*(vec3 Left, vec3 Right)
{
    return v3Multiply(Left, Right);
}

static inline vec4 operator*(vec4 Left, vec4 Right)
{
    return v4Multiply(Left, Right);
}

static inline mat2 operator*(mat2 Left, mat2 Right)
{
    return mat2Multiply(Left, Right);
}

static inline mat3 operator*(mat3 Left, mat3 Right)
{
    return mat3Multiply(Left, Right);
}

static inline mat4 operator*(mat4 Left, mat4 Right)
{
    return mat4Multiply(Left, Right);
}

static inline quat operator*(quat Left, quat Right)
{
    return MulQ(Left, Right);
}

static inline vec2 operator*(vec2 Left, float Right)
{
    return v2Scale(Left, Right);
}

static inline vec3 operator*(vec3 Left, float Right)
{
    return v3Scale(Left, Right);
}

static inline vec4 operator*(vec4 Left, float Right)
{
    return v4Scale(Left, Right);
}

static inline mat2 operator*(mat2 Left, float Right)
{
    return mat2ScaleMatrix(Left, Right);
}

static inline mat3 operator*(mat3 Left, float Right)
{
    return mat3ScaleMatrix(Left, Right);
}

static inline mat4 operator*(mat4 Left, float Right)
{
    return mat4ScaleMatrix(Left, Right);
}

static inline quat operator*(quat Left, float Right)
{
    return quatScale(Left, Right);
}

static inline vec2 operator*(float Left, vec2 Right)
{
    return v2Scale(Right, Left);
}

static inline vec3 operator*(float Left, vec3 Right)
{
    return v3Scale(Right, Left);
}

static inline vec4 operator*(float Left, vec4 Right)
{
    return v4Scale(Right, Left);
}

static inline mat2 operator*(float Left, mat2 Right)
{
    return mat2ScaleMatrix(Right, Left);
}

static inline mat3 operator*(float Left, mat3 Right)
{
    return mat3ScaleMatrix(Right, Left);
}

static inline mat4 operator*(float Left, mat4 Right)
{
    return mat4ScaleMatrix(Right, Left);
}

static inline quat operator*(float Left, quat Right)
{
    return quatScale(Right, Left);
}

static inline vec2 operator*(mat2 Matrix, vec2 Vector)
{
    return mat2Transform(Matrix, Vector);
}

static inline vec3 operator*(mat3 Matrix, vec3 Vector)
{
    return mat3Transform(Matrix, Vector);
}

static inline vec4 operator*(mat4 Matrix, vec4 Vector)
{
    return mat4Transform(Matrix, Vector);
}

static inline vec2 operator/(vec2 Left, vec2 Right)
{
    return v2Divide(Left, Right);
}

static inline vec3 operator/(vec3 Left, vec3 Right)
{
    return v3Divide(Left, Right);
}

static inline vec4 operator/(vec4 Left, vec4 Right)
{
    return v4Divide(Left, Right);
}

static inline vec2 operator/(vec2 Left, float Right)
{
    return v2Reduce(Left, Right);
}

static inline vec3 operator/(vec3 Left, float Right)
{
    return v3Reduce(Left, Right);
}

static inline vec4 operator/(vec4 Left, float Right)
{
    return v4Reduce(Left, Right);
}

static inline mat4 operator/(mat4 Left, float Right)
{
    return mat4Reduce(Left, Right);
}

static inline mat3 operator/(mat3 Left, float Right)
{
    return mat3Reduce(Left, Right);
}

static inline mat2 operator/(mat2 Left, float Right)
{
    return mat2Reduce(Left, Right);
}

static inline quat operator/(quat Left, float Right)
{
    return quatReduce(Left, Right);
}

static inline vec2 &operator+=(vec2 &Left, vec2 Right)
{
    return Left = Left + Right;
}

static inline vec3 &operator+=(vec3 &Left, vec3 Right)
{
    return Left = Left + Right;
}

static inline vec4 &operator+=(vec4 &Left, vec4 Right)
{
    return Left = Left + Right;
}

static inline mat2 &operator+=(mat2 &Left, mat2 Right)
{
    return Left = Left + Right;
}

static inline mat3 &operator+=(mat3 &Left, mat3 Right)
{
    return Left = Left + Right;
}

static inline mat4 &operator+=(mat4 &Left, mat4 Right)
{
    return Left = Left + Right;
}

static inline quat &operator+=(quat &Left, quat Right)
{
    return Left = Left + Right;
}

static inline vec2 &operator-=(vec2 &Left, vec2 Right)
{
    return Left = Left - Right;
}

static inline vec3 &operator-=(vec3 &Left, vec3 Right)
{
    return Left = Left - Right;
}

static inline vec4 &operator-=(vec4 &Left, vec4 Right)
{
    return Left = Left - Right;
}

static inline mat2 &operator-=(mat2 &Left, mat2 Right)
{
    return Left = Left - Right;
}

static inline mat3 &operator-=(mat3 &Left, mat3 Right)
{
    return Left = Left - Right;
}

static inline mat4 &operator-=(mat4 &Left, mat4 Right)
{
    return Left = Left - Right;
}

static inline quat &operator-=(quat &Left, quat Right)
{
    return Left = Left - Right;
}

static inline vec2 &operator*=(vec2 &Left, vec2 Right)
{
    return Left = Left * Right;
}

static inline vec3 &operator*=(vec3 &Left, vec3 Right)
{
    return Left = Left * Right;
}

static inline vec4 &operator*=(vec4 &Left, vec4 Right)
{
    return Left = Left * Right;
}

static inline vec2 &operator*=(vec2 &Left, float Right)
{
    return Left = Left * Right;
}

static inline vec3 &operator*=(vec3 &Left, float Right)
{
    return Left = Left * Right;
}

static inline vec4 &operator*=(vec4 &Left, float Right)
{
    return Left = Left * Right;
}

static inline mat2 &operator*=(mat2 &Left, float Right)
{
    return Left = Left * Right;
}

static inline mat3 &operator*=(mat3 &Left, float Right)
{
    return Left = Left * Right;
}

static inline mat4 &operator*=(mat4 &Left, float Right)
{
    return Left = Left * Right;
}

static inline quat &operator*=(quat &Left, float Right)
{
    return Left = Left * Right;
}

static inline vec2 &operator/=(vec2 &Left, vec2 Right)
{
    return Left = Left / Right;
}

static inline vec3 &operator/=(vec3 &Left, vec3 Right)
{
    return Left = Left / Right;
}

static inline vec4 &operator/=(vec4 &Left, vec4 Right)
{
    return Left = Left / Right;
}

static inline vec2 &operator/=(vec2 &Left, float Right)
{
    return Left = Left / Right;
}

static inline vec3 &operator/=(vec3 &Left, float Right)
{
    return Left = Left / Right;
}

static inline vec4 &operator/=(vec4 &Left, float Right)
{
    return Left = Left / Right;
}

static inline mat4 &operator/=(mat4 &Left, float Right)
{
    return Left = Left / Right;
}

static inline quat &operator/=(quat &Left, float Right)
{
    return Left = Left / Right;
}

static inline Bool operator==(vec2 Left, vec2 Right)
{
    return v2Equal(Left, Right);
}


static inline Bool operator==(vec3 Left, vec3 Right)
{
    return v3Equal(Left, Right);
}

static inline Bool operator==(vec4 Left, vec4 Right)
{
    return v4Equal(Left, Right);
}

static inline Bool operator!=(vec2 Left, vec2 Right)
{
    return !v2Equal(Left, Right);
}

static inline Bool operator!=(vec3 Left, vec3 Right)
{
    return !v3Equal(Left, Right);
}

static inline Bool operator!=(vec4 Left, vec4 Right)
{
    return !v4Equal(Left, Right);
}

static inline vec2 operator-(vec2 In)
{
    
    vec2 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;
    
    return Result;
}

static inline vec3 operator-(vec3 In)
{
    
    vec3 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;
    
    return Result;
}

static inline vec4 operator-(vec4 In)
{
    
    vec4 Result;
#if HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_xor_ps(In.SSE, _mm_set1_ps(-0.0f));
#else
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;
    Result.W = -In.W;
#endif
    
    return Result;
}

static inline bool
operator!=(ivec4 A, ivec4 B)
{
    return(!((A.X == B.X) && (A.Y == B.Y) && (A.Width == B.Width) && (A.Height == B.Height)));
}

static inline 
float NormalizeClamp(float value) 
{
    if (value < 0.0f) {
        return 0.0f;
    } else if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}


static inline bool operator==(mat4 A, mat4 B)
{
    return((A.Columns[0] == B.Columns[0]) && (A.Columns[1] == B.Columns[1]) &&
           (A.Columns[2] == B.Columns[2]) && (A.Columns[3] == B.Columns[3]));
}

static inline bool operator!=(mat4 A, mat4 B)
{
    return((A.Columns[0] != B.Columns[0]) && (A.Columns[1] != B.Columns[1]) &&
           (A.Columns[2] != B.Columns[2]) && (A.Columns[3] != B.Columns[3]));
}

#endif /* __cplusplus*/

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif /* HANDMADE_MATH_H */

